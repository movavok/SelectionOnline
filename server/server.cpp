#include "server.h"

#include <limits>

Server::Server(QObject* parent)
    : QObject(parent)
{
    connect(&m_tcpServer, &QTcpServer::newConnection, this, &Server::onNewConnection);

    m_tickTimer.setInterval(16);
    connect(&m_tickTimer, &QTimer::timeout, this, &Server::onTick);

    m_lobbySlots.resize(MAX_PLAYERS);

    m_clock.start();
}

bool Server::start(const QHostAddress& bind, unsigned short port) {
    if (!m_tcpServer.listen(bind, port)) {
        qWarning() << "listen failed:" << m_tcpServer.errorString();
        return false;
    }

    qInfo() << "listening on" << m_tcpServer.serverAddress().toString()
            << ":" << m_tcpServer.serverPort();

    m_tickTimer.start();
    return true;
}

void Server::onNewConnection() {
    while (QTcpSocket* socket = m_tcpServer.nextPendingConnection()) {
        m_clients.push_back(socket);
        m_inBuffers[socket] = QByteArray();
        socket->setParent(this);

        connect(socket, &QTcpSocket::readyRead, this, &Server::onClientReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);

        qInfo() << "client connected:" << socket->peerAddress().toString() << ":" << socket->peerPort();
    }
}

void Server::onClientReadyRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray& buffer = m_inBuffers[socket];
    buffer.append(socket->readAll());

    QByteArray payload;
    while (tryExtractPayload(buffer, payload)) {
        QDataStream dataStream(payload);
        dataStream.setVersion(QDataStream::Qt_6_5);

        quint16 typeRaw = 0;
        dataStream >> typeRaw;
        MessageType type = MessageType(typeRaw);

        if (type == MessageType::Hello) handleHello(socket, dataStream);
        else if (type == MessageType::Ready) handleReady(socket, dataStream);
        else if (type == MessageType::PlayerConfigUpdate) handlePlayerConfigUpdate(socket, dataStream);
        else if (type == MessageType::StartGame) handleStartGame(socket, dataStream);
        else if (type == MessageType::GameSnapshot) handleGameSnapshot(socket, dataStream);
        else if (type == MessageType::PlayerState) handlePlayerState(socket, dataStream);
        else if (type == MessageType::TileUpdate) handleTileUpdate(socket, dataStream);
        else if (type == MessageType::PlayerHit) handlePlayerHit(socket, dataStream);
        else if (type == MessageType::PickupCollected) handlePickupCollected(socket, dataStream);
        else if (type == MessageType::PlayerAttack) handlePlayerAttack(socket, dataStream);
    }
}

const LobbySlot* Server::findLobbySlotByPlayerId(quint32 playerId) const {
    for (const LobbySlot& slot : m_lobbySlots) {
        if (!slot.connected) continue;
        if (slot.playerId == playerId) return &slot;
    }
    return nullptr;
}

void Server::onClientDisconnected() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    qInfo() << "client disconnected:" << socket->peerAddress().toString() << ":" << socket->peerPort();
    m_clients.removeAll(socket);
    m_inBuffers.remove(socket);

    if (socket == m_host)
        m_host = nullptr;

    releasePlayer(socket);

    ensureHostAssigned();
    broadcastLobbyState();
    broadcastLobbyControl();

    socket->deleteLater();
}

void Server::broadcastGameTimeSync() {
    const qint64 nowMs = m_clock.elapsed();
    const qint64 remainingMs = (m_phaseEndMs > nowMs) ? (m_phaseEndMs - nowMs) : 0;
    qint64 boundedMs = remainingMs;
    if (boundedMs < 0) boundedMs = 0;
    const qint64 maxU32 = qint64(std::numeric_limits<quint32>::max());
    if (boundedMs > maxU32) boundedMs = maxU32;

    const QByteArray payload = makeGameTimeSyncPayload(m_gamePhase, quint32(boundedMs));
    for (QTcpSocket* client : m_clients) {
        if (!client) continue;
        sendPacket(client, payload);
    }
    m_lastTimeSyncMs = nowMs;
}

void Server::onTick() {
    const qint64 nowMs = m_clock.elapsed();

    if (m_gamePhase == GamePhase::Countdown) {
        if (nowMs >= m_phaseEndMs) {
            m_gamePhase = GamePhase::Game;
            m_phaseEndMs = nowMs + qint64(MATCH_SECONDS) * 1000;
            broadcastGameTimeSync();
        }
    } else if (m_gamePhase == GamePhase::Game) {
        if (nowMs >= m_phaseEndMs) {
            m_gamePhase = GamePhase::Idle;
            m_phaseEndMs = 0;
            broadcastGameTimeSync();
        }
    }

    if (m_gamePhase != GamePhase::Idle && (nowMs - m_lastTimeSyncMs) >= TIME_SYNC_INTERVAL_MS)
        broadcastGameTimeSync();
}

int Server::findFreeSlot() const {
    for (int index = 0; index < m_lobbySlots.size(); ++index)
        if (!m_lobbySlots[index].connected) return index;
    return -1;
}

void Server::broadcastLobbyState() {
    const QByteArray payload = makeLobbyStatePayload(m_lobbySlots);
    for (QTcpSocket* client : m_clients) sendPacket(client, payload);
}

quint32 Server::hostPlayerId() const {
    if (!m_host) return 0;
    const QHash<QTcpSocket*, PlayerState>::const_iterator iter = m_playerBySocket.find(m_host);
    if (iter == m_playerBySocket.end()) return 0;
    return iter->playerId;
}

bool Server::canStartGame() const {
    if (hostPlayerId() == 0) return false;

    bool anyConnected = false;
    for (const LobbySlot& slot : m_lobbySlots) {
        if (!slot.connected) continue;
        anyConnected = true;
        if (!slot.ready) return false;
    }

    return anyConnected;
}

void Server::broadcastLobbyControl() {
    const QByteArray payload = makeLobbyControlPayload(canStartGame(), hostPlayerId());
    for (QTcpSocket* client : m_clients) sendPacket(client, payload);
}

void Server::ensureHostAssigned() {
    if (m_host && m_playerBySocket.contains(m_host)) return;
    m_host = nullptr;

    for (QTcpSocket* client : m_clients) {
        if (!client) continue;
        if (m_playerBySocket.contains(client)) {
            m_host = client;
            return;
        }
    }
}

void Server::handleHello(QTcpSocket* socket, QDataStream& in) {
    QString nickname;
    in >> nickname;

    if (m_playerBySocket.contains(socket)) {
        const int slotIndex = m_playerBySocket[socket].slotIndex;
        if (slotIndex >= 0 && slotIndex < m_lobbySlots.size()) {
            m_lobbySlots[slotIndex].nickname = nickname;
        }
        ensureHostAssigned();
        broadcastLobbyState();
        broadcastLobbyControl();
        return;
    }

    const int freeSlot = findFreeSlot();
    if (freeSlot < 0) {
        socket->disconnectFromHost();
        return;
    }

    const quint32 playerId = m_nextPlayerId++;
    m_playerBySocket[socket] = PlayerState{freeSlot, playerId};

    LobbySlot& slot = m_lobbySlots[freeSlot];
    slot.connected = true;
    slot.playerId = playerId;
    slot.nickname = nickname;
    slot.ready = false;

    if (!m_host) m_host = socket;

    sendPacket(socket, makeWelcomePayload(playerId, MAX_PLAYERS));
    ensureHostAssigned();
    broadcastLobbyState();
    broadcastLobbyControl();
}

void Server::handleReady(QTcpSocket* socket, QDataStream& in) {
    bool ready = false;
    in >> ready;

    QHash<QTcpSocket*, PlayerState>::iterator iter = m_playerBySocket.find(socket);
    if (iter == m_playerBySocket.end()) return;

    const int index = iter->slotIndex;
    if (index < 0 || index >= m_lobbySlots.size()) return;

    m_lobbySlots[index].ready = ready;
    broadcastLobbyState();
    broadcastLobbyControl();
}

void Server::handlePlayerConfigUpdate(QTcpSocket* socket, QDataStream& in) {
    quint8 weaponId = 255, abilityId = 255, colorId = 255;
    in >> weaponId >> abilityId >> colorId;

    QHash<QTcpSocket*, PlayerState>::iterator iter = m_playerBySocket.find(socket);
    if (iter == m_playerBySocket.end()) return;

    int index = iter->slotIndex;
    if (index < 0 || index >= m_lobbySlots.size()) return;

    m_lobbySlots[index].weaponId = weaponId;
    m_lobbySlots[index].abilityId = abilityId;
    m_lobbySlots[index].colorId = colorId;

    broadcastLobbyState();
    broadcastLobbyControl();
}

void Server::handleStartGame(QTcpSocket* socket, QDataStream& in) {
    Q_UNUSED(in);

    ensureHostAssigned();
    if (!m_host || socket != m_host) return;
    if (!canStartGame()) return;

    if (m_gamePhase != GamePhase::Idle)
        return;

    const qint64 nowMs = m_clock.elapsed();
    m_gamePhase = GamePhase::Countdown;
    m_phaseEndMs = nowMs + qint64(COUNTDOWN_SECONDS) * 1000;
    m_lastTimeSyncMs = 0;

    const QByteArray payload = makeStartGamePayload();
    for (QTcpSocket* client : m_clients) sendPacket(client, payload);

    broadcastGameTimeSync();
}

void Server::handleGameSnapshot(QTcpSocket* socket, QDataStream& in) {
    quint32 tick = 0;
    QByteArray snapshotBytes;
    in >> tick >> snapshotBytes;

    ensureHostAssigned();
    if (!m_host || socket != m_host) return;

    const QByteArray payload = makeGameSnapshotPayload(tick, snapshotBytes);
    for (QTcpSocket* client : m_clients) {
        if (!client || client == socket) continue;
        sendPacket(client, payload);
    }
}

void Server::handlePlayerState(QTcpSocket* socket, QDataStream& in) {
    quint32 tick = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    quint16 hp = 0, maxHp = 0;
    quint8 activeItemKind = 0;
    quint8 activeResourceType = 0;
    float aimDirX = 1.0f;
    float aimDirY = 0.0f;
    in >> tick >> posX >> posY >> hp >> maxHp >> activeItemKind >> activeResourceType >> aimDirX >> aimDirY;

    const QHash<QTcpSocket*, PlayerState>::const_iterator iter = m_playerBySocket.find(socket);
    if (iter == m_playerBySocket.end()) return;
    const quint32 playerId = iter->playerId;
    if (playerId == 0) return;

    const LobbySlot* slot = findLobbySlotByPlayerId(playerId);
    const QString nickname = slot ? slot->nickname : QString();
    const quint8 colorId = slot ? slot->colorId : quint8(255);

    const QByteArray payload = makePlayerStateBroadcastPayload(playerId, tick, posX, posY, hp, maxHp, nickname,
                                                               colorId, activeItemKind, activeResourceType,
                                                               aimDirX, aimDirY);
    for (QTcpSocket* client : m_clients) {
        if (!client || client == socket) continue;
        sendPacket(client, payload);
    }
}

void Server::handleTileUpdate(QTcpSocket* socket, QDataStream& in) {
    qint16 tileX = 0, tileY = 0;
    quint8 tileType = 0;
    in >> tileX >> tileY >> tileType;

    if (!m_playerBySocket.contains(socket)) return;

    const QByteArray payload = makeTileUpdatePayload(tileX, tileY, tileType);
    for (QTcpSocket* client : m_clients) {
        if (!client || client == socket) continue;
        sendPacket(client, payload);
    }
}

void Server::handlePlayerHit(QTcpSocket* socket, QDataStream& in) {
    quint32 targetPlayerId = 0;
    quint16 damage = 0;
    in >> targetPlayerId >> damage;

    const QHash<QTcpSocket*, PlayerState>::const_iterator attackerIter = m_playerBySocket.find(socket);
    if (attackerIter == m_playerBySocket.end()) return;
    const quint32 attackerPlayerId = attackerIter->playerId;
    if (attackerPlayerId == 0 || targetPlayerId == 0) return;
    if (attackerPlayerId == targetPlayerId) return;
    if (damage == 0) return;

    QTcpSocket* targetSocket = nullptr;
    for (auto iter = m_playerBySocket.constBegin(); iter != m_playerBySocket.constEnd(); ++iter) {
        if (iter->playerId == targetPlayerId) {
            targetSocket = iter.key();
            break;
        }
    }
    if (!targetSocket) return;

    sendPacket(targetSocket, makePlayerHitNotifyPayload(attackerPlayerId, targetPlayerId, damage));
}

void Server::handlePickupCollected(QTcpSocket* socket, QDataStream& in) {
    qint16 tileX = 0;
    qint16 tileY = 0;
    in >> tileX >> tileY;

    if (!m_playerBySocket.contains(socket)) return;

    const QByteArray payload = makePickupCollectedPayload(tileX, tileY);
    for (QTcpSocket* client : m_clients) {
        if (!client || client == socket) continue;
        sendPacket(client, payload);
    }
}

void Server::handlePlayerAttack(QTcpSocket* socket, QDataStream& in) {
    quint32 tick = 0;
    float dirX = 0.0f;
    float dirY = 0.0f;
    in >> tick >> dirX >> dirY;

    const QHash<QTcpSocket*, PlayerState>::const_iterator iter = m_playerBySocket.find(socket);
    if (iter == m_playerBySocket.end()) return;
    const quint32 playerId = iter->playerId;
    if (playerId == 0) return;

    const QByteArray payload = makePlayerAttackBroadcastPayload(playerId, tick, dirX, dirY);
    for (QTcpSocket* client : m_clients) {
        if (!client || client == socket) continue;
        sendPacket(client, payload);
    }
}

void Server::releasePlayer(QTcpSocket* socket) {
    QHash<QTcpSocket*, PlayerState>::iterator iter = m_playerBySocket.find(socket);
    if (iter == m_playerBySocket.end()) return;

    const int removedIndex = iter->slotIndex;

    m_playerBySocket.erase(iter);

    if (removedIndex < 0 || removedIndex >= m_lobbySlots.size()) {
        if (removedIndex >= 0 && removedIndex < m_lobbySlots.size())
            m_lobbySlots[removedIndex] = LobbySlot();
        broadcastLobbyState();
        return;
    }

    compactSlotsFrom(removedIndex);
}

void Server::compactSlotsFrom(int removedIndex) {
    if (removedIndex < 0 || removedIndex >= m_lobbySlots.size()) return;

    for (int index = removedIndex; index + 1 < m_lobbySlots.size(); ++index)
        m_lobbySlots[index] = m_lobbySlots[index + 1];

    m_lobbySlots[m_lobbySlots.size() - 1] = LobbySlot();

    for (QHash<QTcpSocket*, PlayerState>::iterator iter = m_playerBySocket.begin(); iter != m_playerBySocket.end(); ++iter) {
        if (iter->slotIndex > removedIndex)
            --(iter->slotIndex);
    }

    broadcastLobbyState();
}
