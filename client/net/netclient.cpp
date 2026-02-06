#include "netclient.h"

NetClient::NetClient(QObject* parent)
    : QObject(parent)
{
    connect(&m_socket, &QTcpSocket::connected, this, &NetClient::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected, this, &NetClient::onDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &NetClient::onReadyRead);

    connect(&m_socket, &QTcpSocket::errorOccurred, this, &NetClient::onSocketErrorOccurred);
}

void NetClient::connectToServer(const QString& ip, unsigned short port) {
    m_buffer.clear();
    m_socket.abort();
    m_socket.connectToHost(ip, port);
}

void NetClient::disconnectFromServer() {
    m_socket.disconnectFromHost();
}

void NetClient::sendHello(const QString& nickname) {
    sendPacket(&m_socket, makeHelloPayload(nickname));
}

void NetClient::sendReady(bool ready) {
    sendPacket(&m_socket, makeReadyPayload(ready));
}

void NetClient::sendPlayerConfigUpdate(quint8 weaponId, quint8 abilityId, quint8 colorId) {
    sendPacket(&m_socket, makePlayerConfigUpdatePayload(weaponId, abilityId, colorId));
}

void NetClient::sendStartGame() {
    sendPacket(&m_socket, makeStartGamePayload());
}

void NetClient::sendGameSnapshot(quint32 tick, const QByteArray& snapshotBytes) {
    sendPacket(&m_socket, makeGameSnapshotPayload(tick, snapshotBytes));
}

void NetClient::sendPlayerState(quint32 tick, float posX, float posY, quint16 hp, quint16 maxHp, quint8 activeItemKind,
                                quint8 activeResourceType, float aimDirX, float aimDirY) {
    sendPacket(&m_socket,
               makePlayerStateUpdatePayload(tick, posX, posY, hp, maxHp, activeItemKind, activeResourceType,
                                            aimDirX, aimDirY));
}

void NetClient::sendTileUpdate(qint16 tileX, qint16 tileY, quint8 tileType) {
    sendPacket(&m_socket, makeTileUpdatePayload(tileX, tileY, tileType));
}

void NetClient::sendPlayerHit(quint32 targetPlayerId, quint16 damage) {
    sendPacket(&m_socket, makePlayerHitRequestPayload(targetPlayerId, damage));
}

void NetClient::sendPickupCollected(qint16 tileX, qint16 tileY) {
    sendPacket(&m_socket, makePickupCollectedPayload(tileX, tileY));
}

void NetClient::sendPlayerAttack(quint32 tick, float dirX, float dirY) {
    sendPacket(&m_socket, makePlayerAttackRequestPayload(tick, dirX, dirY));
}

void NetClient::onConnected() { emit connected(); }
void NetClient::onDisconnected() { emit disconnected(); }

void NetClient::onReadyRead() {
    m_buffer.append(m_socket.readAll());

    QByteArray payload;
    while (tryExtractPayload(m_buffer, payload)) {
        QDataStream dataStream(payload);
        dataStream.setVersion(QDataStream::Qt_6_5);

        quint16 typeRaw = 0;
        dataStream >> typeRaw;
        MessageType type = MessageType(typeRaw);

        if (type == MessageType::Welcome) {
            quint32 playerId = 0;
            quint8 maxPlayers = 0;

            dataStream >> playerId >> maxPlayers;
            emit welcomeReceived(playerId, maxPlayers);
        } else if (type == MessageType::LobbyState) {
            quint8 count = 0;
            dataStream >> count;

            QVector<LobbySlot> lobbySlots;
            lobbySlots.resize(count);

            for (int index = 0; index < count; ++index) {
                dataStream >> lobbySlots[index].connected
                           >> lobbySlots[index].playerId >> lobbySlots[index].nickname
                           >> lobbySlots[index].weaponId >> lobbySlots[index].abilityId >> lobbySlots[index].colorId
                           >> lobbySlots[index].ready;
            }

            emit lobbyStateReceived(lobbySlots);
        } else if (type == MessageType::LobbyControl) {
            bool canStart = false;
            quint32 hostPlayerId = 0;
            dataStream >> canStart >> hostPlayerId;
            emit lobbyControlReceived(canStart, hostPlayerId);
        } else if (type == MessageType::StartGame) {
            emit startGameReceived();
        } else if (type == MessageType::GameSnapshot) {
            quint32 tick = 0;
            QByteArray snapshotBytes;
            dataStream >> tick >> snapshotBytes;
            emit gameSnapshotReceived(tick, snapshotBytes);
        } else if (type == MessageType::PlayerState) {
            RemotePlayerStateUpdate update;
            dataStream >> update.playerId >> update.tick >> update.posX >> update.posY >> update.hp >> update.maxHp
                       >> update.nickname >> update.colorId >> update.activeItemKind >> update.activeResourceType
                       >> update.aimDirX >> update.aimDirY;
            emit playerStateReceived(update);
        } else if (type == MessageType::TileUpdate) {
            qint16 tileX = 0, tileY = 0;
            quint8 tileType = 0;
            dataStream >> tileX >> tileY >> tileType;
            emit tileUpdateReceived(tileX, tileY, tileType);
        } else if (type == MessageType::PlayerHit) {
            quint32 attackerPlayerId = 0, targetPlayerId = 0;
            quint16 damage = 0;
            dataStream >> attackerPlayerId >> targetPlayerId >> damage;
            emit playerHitReceived(attackerPlayerId, targetPlayerId, damage);
        } else if (type == MessageType::PickupCollected) {
            qint16 tileX = 0, tileY = 0;
            dataStream >> tileX >> tileY;
            emit pickupCollectedReceived(tileX, tileY);
        } else if (type == MessageType::PlayerAttack) {
            quint32 attackerPlayerId = 0;
            quint32 tick = 0;
            float dirX = 0.0f;
            float dirY = 0.0f;
            dataStream >> attackerPlayerId >> tick >> dirX >> dirY;
            emit playerAttackReceived(attackerPlayerId, tick, dirX, dirY);
        } else if (type == MessageType::GameTimeSync) {
            quint8 phase = 0;
            quint32 msLeft = 0;
            dataStream >> phase >> msLeft;
            emit gameTimeSyncReceived(phase, msLeft);
        }
    }
}

void NetClient::onSocketErrorOccurred(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    emit errorText(m_socket.errorString());
}
