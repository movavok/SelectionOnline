#ifndef PACKET_H
#define PACKET_H

#include <QTcpSocket>

#include "protocol.h"

static constexpr int PACKET_HEADER_SIZE = sizeof(quint32);

static inline void sendPacket(QTcpSocket* socket, const QByteArray& payload) {
    QByteArray packetBuffer;
    QDataStream dataStream(&packetBuffer, QIODevice::WriteOnly);
    dataStream.setVersion(QDataStream::Qt_6_5);

    dataStream << quint32(payload.size());
    packetBuffer.append(payload);
    socket->write(packetBuffer);
}

static inline bool tryExtractPayload(QByteArray& receiveBuffer, QByteArray& payload) {
    if (receiveBuffer.size() < PACKET_HEADER_SIZE)
        return false;

    QDataStream dataStream(receiveBuffer);
    dataStream.setVersion(QDataStream::Qt_6_5);

    quint32 payloadSize = 0;
    dataStream >> payloadSize;

    if (payloadSize > MAX_PACKET_SIZE)
        return false;

    if (receiveBuffer.size() < PACKET_HEADER_SIZE + int(payloadSize))
        return false;

    payload = receiveBuffer.mid(PACKET_HEADER_SIZE, payloadSize);
    receiveBuffer.remove(0, PACKET_HEADER_SIZE + payloadSize);

    return true;
}

#endif // PACKET_H
