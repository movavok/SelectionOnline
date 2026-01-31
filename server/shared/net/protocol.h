#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QDataStream>
#include <QString>

enum class MessageType : quint16 {
    Hello = 1,
    Welcome = 2
};

static constexpr quint32 MAX_PACKET_SIZE = 64 * 1024;

static inline QByteArray makeHelloPayload(const QString& nickname) {
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_6_5);

    payloadStream << quint16(MessageType::Hello);
    payloadStream << nickname;
    return payload;
}

static inline QByteArray makeWelcomePayload(quint32 playerId, quint8 maxPlayers) {
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_6_5);

    payloadStream << quint16(MessageType::Welcome);
    payloadStream << playerId;
    payloadStream << maxPlayers;
    return payload;
}

#endif // PROTOCOL_H
