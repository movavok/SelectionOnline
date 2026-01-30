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
    const QByteArray line = "HELLO " + nickname.toUtf8() + "\n";
    m_socket.write(line);
}

void NetClient::onConnected() { emit connected(); }
void NetClient::onDisconnected() { emit disconnected(); }

void NetClient::onReadyRead() {
    m_buffer.append(m_socket.readAll());

    const QString text = QString::fromUtf8(m_buffer);
    m_buffer.clear();
    emit textReceived(text);
}

void NetClient::onSocketErrorOccurred(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    emit errorText(m_socket.errorString());
}
