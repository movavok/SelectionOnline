#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QStringList>

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
};

#endif // SERVER_H
