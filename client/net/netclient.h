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

signals:
    void connected();
    void disconnected();
    void errorText(const QString&);

    void welcomeReceived(quint32 playerId, quint8 maxPlayers);
    void lobbyStateReceived(const QVector<LobbySlot>&);
    void lobbyControlReceived(bool canStart, quint32 hostPlayerId);
    void startGameReceived();

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
