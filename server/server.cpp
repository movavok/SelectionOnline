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
        socket->setParent(this);

        connect(socket, &QTcpSocket::readyRead, this, &Server::onClientReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);

        qInfo() << "client connected:" << socket->peerAddress().toString() << ":" << socket->peerPort();
    }
}

void Server::onClientReadyRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    const QByteArray data = socket->readAll();
    qInfo() << "rx bytes:" << data.size();

    socket->write("OK\n");
}

void Server::onClientDisconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    qInfo() << "client disconnected:" << socket->peerAddress().toString() << ":" << socket->peerPort();
    m_clients.removeAll(socket);
    socket->deleteLater();
}

void Server::onTick() {}
