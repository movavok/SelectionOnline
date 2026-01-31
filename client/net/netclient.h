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

signals:
    void connected();
    void disconnected();
    void errorText(const QString&);
    void welcomeReceived(quint32 playerId, quint8 maxPlayers);

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
