#include "netclient.h"

NetClient::NetClient(QObject* parent)
    : QObject(parent)
{
    connect(&m_socket, &QTcpSocket::connected, this, &NetClient::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &NetClient::onDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &NetClient::onReadyRead);

    connect(&m_socket, &QTcpSocket::errorOccurred, this, &NetClient::onSocketErrorOccurred);
}

void NetClient::connectToServer(const QString& ip, unsigned short port) {
    m_buffer.clear();
    m_socket.abort();
    m_socket.connectToHost(ip, port);
}

void NetClient::disconnectFromServer() {
    m_socket.disconnectFromHost();
}

void NetClient::sendHello(const QString& nickname) {
    sendPacket(&m_socket, makeHelloPayload(nickname));
}

void NetClient::sendReady(bool ready) {
    sendPacket(&m_socket, makeReadyPayload(ready));
}

void NetClient::onConnected() { emit connected(); }
void NetClient::onDisconnected() { emit disconnected(); }

void NetClient::onReadyRead() {
    m_buffer.append(m_socket.readAll());

    QByteArray payload;
    while (tryExtractPayload(m_buffer, payload)) {
        QDataStream dataStream(payload);
        dataStream.setVersion(QDataStream::Qt_6_5);

        quint16 typeRaw = 0;
        dataStream >> typeRaw;
        MessageType type = MessageType(typeRaw);

        if (type == MessageType::Welcome) {
            quint32 playerId = 0;
            quint8 maxPlayers = 0;

            dataStream >> playerId >> maxPlayers;
            emit welcomeReceived(playerId, maxPlayers);
        } else if (type == MessageType::LobbyState) {
            quint8 count = 0;
            dataStream >> count;

            QVector<LobbySlot> lobbySlots;
            lobbySlots.resize(count);

            for (int index = 0; index < count; ++index)
                dataStream >> lobbySlots[index].connected >> lobbySlots[index].playerId
                           >> lobbySlots[index].nickname >> lobbySlots[index].ready;

            emit lobbyStateReceived(lobbySlots);
        }
    }
}

void NetClient::onSocketErrorOccurred(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    emit errorText(m_socket.errorString());
}
