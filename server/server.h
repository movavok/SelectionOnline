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

    struct PlayerState {
        int slotIndex = -1;
        quint32 playerId = 0;
    };

    QHash<QTcpSocket*, PlayerState> m_playerBySocket;
    QVector<LobbySlot> m_lobbySlots;

    void broadcastLobbyState();
    int findFreeSlot() const;
    void assignPlayerToSlot(QTcpSocket*, const QString& nickname);
    void releasePlayer(QTcpSocket*);
    void handleHello(QTcpSocket*, QDataStream&);
    void handleReady(QTcpSocket*, QDataStream&);
};

#endif // SERVER_H
