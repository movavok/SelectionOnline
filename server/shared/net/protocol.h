#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QVector>
#include <QColor>

enum class MessageType : quint16 {
    Hello = 1,
    Welcome = 2,
    LobbyState = 3,
    Ready = 4
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

struct LobbySlot {
    bool connected = false;
    quint32 playerId = 0;
    QString nickname;
    bool ready = false;
};

static inline QByteArray makeLobbyStatePayload(const QVector<LobbySlot>& lobbySlots) {
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_6_5);

    payloadStream << quint16(MessageType::LobbyState);
    payloadStream << quint8(lobbySlots.size());

    for (const LobbySlot& slot : lobbySlots)
        payloadStream << slot.connected << slot.playerId << slot.nickname << slot.ready;

    return payload;
}

static inline QByteArray makeReadyPayload(bool ready) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::Ready);
    out << ready;
    return payload;
}

#endif // PROTOCOL_H
