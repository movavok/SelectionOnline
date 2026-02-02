#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QVector>

enum class MessageType : quint16 {
    Hello = 1,
    Welcome = 2,
    LobbyState = 3,
    Ready = 4,
    PlayerConfigUpdate = 5,
    LobbyControl = 6,
    StartGame = 7,
    GameSnapshot = 8,
    PlayerState = 9,
    TileUpdate = 10,
    PlayerHit = 11
};

static constexpr quint32 MAX_PACKET_SIZE = 64 * 1024;

static inline QByteArray makeHelloPayload(const QString& nickname) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::Hello);
    out << nickname;
    return payload;
}

static inline QByteArray makeWelcomePayload(quint32 playerId, quint8 maxPlayers) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::Welcome);
    out << playerId << maxPlayers;
    return payload;
}

struct LobbySlot {
    bool connected = false;
    quint32 playerId = 0;
    QString nickname;
    quint8 weaponId = 255;
    quint8 abilityId = 255;
    quint8 colorId = 255;
    bool ready = false;
};

static inline QByteArray makeLobbyStatePayload(const QVector<LobbySlot>& lobbySlots) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::LobbyState);
    out << quint8(lobbySlots.size());

    for (const LobbySlot& slot : lobbySlots)
        out << slot.connected
            << slot.playerId << slot.nickname
            << slot.weaponId << slot.abilityId << slot.colorId
            << slot.ready;

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

static inline QByteArray makePlayerConfigUpdatePayload(quint8 weaponId, quint8 abilityId, quint8 colorId) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::PlayerConfigUpdate);
    out << weaponId << abilityId << colorId;
    return payload;
}

static inline QByteArray makeLobbyControlPayload(bool canStart, quint32 hostPlayerId) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::LobbyControl);
    out << canStart;
    out << hostPlayerId;
    return payload;
}

static inline QByteArray makeStartGamePayload() {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::StartGame);
    return payload;
}

static inline QByteArray makeGameSnapshotPayload(quint32 tick, const QByteArray& snapshotBytes) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::GameSnapshot);
    out << tick;
    out << snapshotBytes;
    return payload;
}

static inline QByteArray makePlayerStateUpdatePayload(quint32 tick, float posX, float posY, quint16 hp, quint16 maxHp) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::PlayerState);
    out << tick;
    out << posX << posY;
    out << hp << maxHp;
    return payload;
}

static inline QByteArray makePlayerStateBroadcastPayload(quint32 playerId,
                                                        quint32 tick,
                                                        float posX,
                                                        float posY,
                                                        quint16 hp,
                                                        quint16 maxHp,
                                                        const QString& nickname,
                                                        quint8 colorId) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::PlayerState);
    out << playerId;
    out << tick;
    out << posX << posY;
    out << hp << maxHp;
    out << nickname;
    out << colorId;
    return payload;
}

static inline QByteArray makeTileUpdatePayload(qint16 tileX, qint16 tileY, quint8 tileType) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::TileUpdate);
    out << tileX << tileY << tileType;
    return payload;
}

static inline QByteArray makePlayerHitRequestPayload(quint32 targetPlayerId, quint16 damage) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::PlayerHit);
    out << targetPlayerId << damage;
    return payload;
}

static inline QByteArray makePlayerHitNotifyPayload(quint32 attackerPlayerId, quint32 targetPlayerId, quint16 damage) {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);

    out << quint16(MessageType::PlayerHit);
    out << attackerPlayerId << targetPlayerId << damage;
    return payload;
}

#endif // PROTOCOL_H
