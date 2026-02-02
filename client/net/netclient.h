#ifndef NETCLIENT_H
#define NETCLIENT_H

#include <QObject>
#include <QTcpSocket>

#include "../../server/shared/net/packet.h"
#include "../../server/shared/net/protocol.h"

class NetClient : public QObject
{
    Q_OBJECT
public:
    explicit NetClient(QObject* parent = nullptr);

    void connectToServer(const QString& ip, unsigned short port);
    void disconnectFromServer();

    void sendHello(const QString& nickname);
    void sendReady(bool);
    void sendPlayerConfigUpdate(quint8 weaponId, quint8 abilityId, quint8 colorId);
    void sendStartGame();
    void sendGameSnapshot(quint32 tick, const QByteArray& snapshotBytes);
    void sendPlayerState(quint32 tick, float posX, float posY, quint16 hp, quint16 maxHp);
    void sendTileUpdate(qint16 tileX, qint16 tileY, quint8 tileType);
    void sendPlayerHit(quint32 targetPlayerId, quint16 damage);

signals:
    void connected();
    void disconnected();
    void errorText(const QString&);

    void welcomeReceived(quint32 playerId, quint8 maxPlayers);
    void lobbyStateReceived(const QVector<LobbySlot>&);
    void lobbyControlReceived(bool canStart, quint32 hostPlayerId);
    void startGameReceived();
    void gameSnapshotReceived(quint32 tick, const QByteArray& snapshotBytes);
    void playerStateReceived(quint32 playerId, quint32 tick, float posX, float posY, quint16 hp, quint16 maxHp, const QString& nickname, quint8 colorId);
    void tileUpdateReceived(qint16 tileX, qint16 tileY, quint8 tileType);
    void playerHitReceived(quint32 attackerPlayerId, quint32 targetPlayerId, quint16 damage);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketErrorOccurred(QAbstractSocket::SocketError);

private:
    QTcpSocket m_socket;
    QByteArray m_buffer;
};

#endif // NETCLIENT_H
