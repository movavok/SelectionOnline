#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QStringList>

#include "shared/net/packet.h"
#include "shared/net/protocol.h"

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(QObject* parent = nullptr);

    bool start(const QHostAddress& bind, unsigned short port);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();
    void onTick();

private:
    QTcpServer m_tcpServer;
    QList<QTcpSocket*> m_clients;
    QTimer m_tickTimer;

    QHash<QTcpSocket*, QByteArray> m_inBuffers;
    quint32 m_nextPlayerId = 1;
    static constexpr quint8 MAX_PLAYERS = 10;
};

#endif // SERVER_H
