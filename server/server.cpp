#include "server.h"

Server::Server(QObject* parent)
    : QObject(parent)
{
    connect(&m_tcpServer, &QTcpServer::newConnection, this, &Server::onNewConnection);

    m_tickTimer.setInterval(16);
    connect(&m_tickTimer, &QTimer::timeout, this, &Server::onTick);

    m_lobbySlots.resize(MAX_PLAYERS);
}

bool Server::start(const QHostAddress& bind, unsigned short port) {
    if (!m_tcpServer.listen(bind, port)) {
        qWarning() << "listen failed:" << m_tcpServer.errorString();
        return false;
    }

    qInfo() << "listening on" << m_tcpServer.serverAddress().toString()
            << ":" << m_tcpServer.serverPort();

    m_tickTimer.start();
    return true;
}

void Server::onNewConnection() {
    while (QTcpSocket* socket = m_tcpServer.nextPendingConnection()) {
        m_clients.push_back(socket);
        m_inBuffers[socket] = QByteArray();
        socket->setParent(this);

        connect(socket, &QTcpSocket::readyRead, this, &Server::onClientReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);

        qInfo() << "client connected:" << socket->peerAddress().toString() << ":" << socket->peerPort();
    }
}

void Server::onClientReadyRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray& buffer = m_inBuffers[socket];
    buffer.append(socket->readAll());

    QByteArray payload;
    while (tryExtractPayload(buffer, payload)) {
        QDataStream dataStream(payload);
        dataStream.setVersion(QDataStream::Qt_6_5);

        quint16 typeRaw = 0;
        dataStream >> typeRaw;
        MessageType type = MessageType(typeRaw);

        if (type == MessageType::Hello) handleHello(socket, dataStream);
        else if (type == MessageType::Ready) handleReady(socket, dataStream);
    }
}

void Server::onClientDisconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    qInfo() << "client disconnected:" << socket->peerAddress().toString() << ":" << socket->peerPort();
    m_clients.removeAll(socket);
    m_inBuffers.remove(socket);

    releasePlayer(socket);
    broadcastLobbyState();

    socket->deleteLater();
}

void Server::onTick() {}

int Server::findFreeSlot() const {
    for (int index = 0; index < m_lobbySlots.size(); ++index)
        if (!m_lobbySlots[index].connected) return index;
    return -1;
}

void Server::broadcastLobbyState() {
    const QByteArray payload = makeLobbyStatePayload(m_lobbySlots);
    for (QTcpSocket* client : m_clients) sendPacket(client, payload);
}

void Server::handleHello(QTcpSocket* socket, QDataStream& in) {
    QString nickname;
    in >> nickname;

    if (m_playerBySocket.contains(socket)) {
        int idx = m_playerBySocket[socket].slotIndex;
        if (idx >= 0 && idx < m_lobbySlots.size()) {
            m_lobbySlots[idx].nickname = nickname;
        }
        broadcastLobbyState();
        return;
    }

    const int freeSlot = findFreeSlot();
    if (freeSlot < 0) {
        socket->disconnectFromHost();
        return;
    }

    const quint32 playerId = m_nextPlayerId++;
    m_playerBySocket[socket] = PlayerState{freeSlot, playerId};

    LobbySlot& slot = m_lobbySlots[freeSlot];
    slot.connected = true;
    slot.playerId = playerId;
    slot.nickname = nickname;
    slot.ready = false;

    sendPacket(socket, makeWelcomePayload(playerId, MAX_PLAYERS));
    broadcastLobbyState();
}

void Server::handleReady(QTcpSocket* socket, QDataStream& in) {
    bool ready = false;
    in >> ready;

    QHash<QTcpSocket*, PlayerState>::iterator iter = m_playerBySocket.find(socket);
    if (iter == m_playerBySocket.end()) return;

    const int index = iter->slotIndex;
    if (index < 0 || index >= m_lobbySlots.size()) return;

    m_lobbySlots[index].ready = ready;
    broadcastLobbyState();
}

void Server::releasePlayer(QTcpSocket* socket) {
    QHash<QTcpSocket*, PlayerState>::iterator iter = m_playerBySocket.find(socket);
    if (iter == m_playerBySocket.end()) return;

    const int removedIndex = iter->slotIndex;

    m_playerBySocket.erase(iter);

    if (removedIndex < 0 || removedIndex >= m_lobbySlots.size()) {
        if (removedIndex >= 0 && removedIndex < m_lobbySlots.size())
            m_lobbySlots[removedIndex] = LobbySlot();
        broadcastLobbyState();
        return;
    }

    compactSlotsFrom(removedIndex);
}

void Server::compactSlotsFrom(int removedIndex) {
    if (removedIndex < 0 || removedIndex >= m_lobbySlots.size()) return;

    for (int index = removedIndex; index + 1 < m_lobbySlots.size(); ++index)
        m_lobbySlots[index] = m_lobbySlots[index + 1];

    m_lobbySlots[m_lobbySlots.size() - 1] = LobbySlot();

    for (QHash<QTcpSocket*, PlayerState>::iterator iter = m_playerBySocket.begin(); iter != m_playerBySocket.end(); ++iter) {
        if (iter->slotIndex > removedIndex)
            --(iter->slotIndex);
    }

    broadcastLobbyState();
}
