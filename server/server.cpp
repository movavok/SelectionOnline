#include "server.h"

Server::Server(QObject* parent)
    : QObject(parent)
{
    connect(&m_tcpServer, &QTcpServer::newConnection, this, &Server::onNewConnection);

    m_tickTimer.setInterval(16);
    connect(&m_tickTimer, &QTimer::timeout, this, &Server::onTick);
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

        if (type == MessageType::Hello) {
            QString nickname;
            dataStream >> nickname;

            quint32 playerId = m_nextPlayerId++;
            sendPacket(socket, makeWelcomePayload(playerId, MAX_PLAYERS));
        }
    }
}

void Server::onClientDisconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    qInfo() << "client disconnected:" << socket->peerAddress().toString() << ":" << socket->peerPort();
    m_clients.removeAll(socket);
    m_inBuffers.remove(socket);
    socket->deleteLater();
}

void Server::onTick() {}
