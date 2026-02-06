#include "fakeserver.h"

void FakeServer::update(float dt) { m_game.update(dt); }

void FakeServer::setPlayerInput(MoveDirection direction, bool pressed) {
    m_game.setPlayerInput(direction, pressed);
}

void FakeServer::setActiveSlot(int slotIndex) {
    if (Player* player = m_game.getPlayer())
        player->setActiveSlot(slotIndex);
}

void FakeServer::startAiming() {
    if (Player* player = m_game.getPlayer())
        player->startAiming();
}

void FakeServer::stopAiming(const QPointF& attackDirWorld) {
    if (Player* player = m_game.getPlayer())
        player->stopAiming(attackDirWorld);
}

bool FakeServer::tryPlaceTile(const QPoint& tilePos) {
    return m_game.tryPlaceTile(tilePos);
}

Game& FakeServer::game() { return m_game; }
const Game& FakeServer::game() const { return m_game; }

quint32 FakeServer::entityIdFor(const Entity* entity) const {
    if (!entity) return 0;
    QHash<const Entity*, quint32>::const_iterator iter = m_entityIds.find(entity);
    if (iter == m_entityIds.end()) return 0;
    return iter.value();
}

quint32 FakeServer::ensureIdFor(const Entity* entity) {
    if (!entity) return 0;

    QHash<const Entity*, quint32>::iterator iter = m_entityIds.find(entity);
    if (iter != m_entityIds.end()) return iter.value();

    const quint32 entityId = m_nextId++;
    m_entityIds.insert(entity, entityId);
    return entityId;
}

void FakeServer::ensureEntityIds() {
    const QList<Entity*>& entities = m_game.getEntities();
    for (const Entity* entity : entities) ensureIdFor(entity);
}

void FakeServer::fillEntitySnapshotCommon(EntitySnapshot& entitySnapshot, const Entity* entity) const {
    entitySnapshot.alive = entity->isAlive();
    entitySnapshot.pos = entity->getPosition();
    entitySnapshot.prevPos = entity->getPrevPosition();
    entitySnapshot.aimDir = QPointF(0, 0);
    entitySnapshot.radius = entity->getRadius();
    entitySnapshot.hp = entity->getCurrentHp();
    entitySnapshot.maxHp = entity->getMaxHp();
}

void FakeServer::fillPlayerSnapshot(EntitySnapshot& entitySnapshot, const Player& player) const {
    entitySnapshot.nickname = player.getNickname();
    entitySnapshot.uiColor = player.getUiColor();
}

void FakeServer::fillLocalPlayerSnapshot(EntitySnapshot& entitySnapshot, const Player& localPlayer, const QPointF& mouseScene) {
    QPointF aimDir = mouseScene - localPlayer.getPosition();
    const double aimLen = std::hypot(aimDir.x(), aimDir.y());
    if (aimLen > 0.0001) aimDir /= aimLen;
    else aimDir = QPointF(1.0, 0.0);
    entitySnapshot.aimDir = aimDir;

    const Weapon* weapon = localPlayer.getInventory().getActiveWeapon();
    entitySnapshot.showAttackIndicator = (weapon && localPlayer.getAttackState() != Player::AttackState::Idle);
    entitySnapshot.canAttack = localPlayer.canAttack();

    if (entitySnapshot.showAttackIndicator) {
        const QPainterPath attackShapeWorld = m_game.getPlayerAttackShape(mouseScene);

        const QPointF center(localPlayer.getRadius() / 2.0f, localPlayer.getRadius() / 2.0f);
        QTransform toLocal;
        toLocal.translate(-localPlayer.getPosition().x() + center.x(),
                          -localPlayer.getPosition().y() + center.y());

        entitySnapshot.attackIndicatorLocalPath = toLocal.map(attackShapeWorld);
    }

    if (localPlayer.getInventory().isActiveResource()) {
        const Tile::TileType tileType = localPlayer.getInventory().getActiveResourceType();
        const TileVisual& visual = tileVisual(tileType);
        if (!visual.sprite.isNull())
            entitySnapshot.slotIndicatorPixmap = visual.sprite.scaled(10, 10, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    } else if (Weapon* activeWeapon = localPlayer.getInventory().getActiveWeapon())
        entitySnapshot.slotIndicatorPixmap = activeWeapon->getSprite().scaled(
                                             activeWeapon->getSpriteSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    entitySnapshot.showSlotIndicator = !entitySnapshot.slotIndicatorPixmap.isNull();
}

EntitySnapshot FakeServer::makeEntitySnapshot(Entity* entity, const Player* localPlayer, const QPointF& mouseScene) {
    EntitySnapshot entitySnapshot;
    entitySnapshot.id = ensureIdFor(entity);

    Player* player = dynamic_cast<Player*>(entity);
    entitySnapshot.kind = player ? SnapshotEntityKind::Player : SnapshotEntityKind::Enemy;

    fillEntitySnapshotCommon(entitySnapshot, entity);

    if (player)
        fillPlayerSnapshot(entitySnapshot, *player);

    entitySnapshot.isLocalPlayer = (localPlayer && entity == localPlayer);
    if (entitySnapshot.isLocalPlayer)
        fillLocalPlayerSnapshot(entitySnapshot, *localPlayer, mouseScene);

    return entitySnapshot;
}

WorldSnapshot FakeServer::makeSnapshot(const QPointF& mouseScene) {
    ensureEntityIds();

    WorldSnapshot snapshot;
    snapshot.entities.reserve(m_game.getEntities().size());

    Player* localPlayer = m_game.getPlayer();
    for (Entity* entity : m_game.getEntities())
        snapshot.entities.push_back(makeEntitySnapshot(entity, localPlayer, mouseScene));

    return snapshot;
}

void FakeServer::setExternalCollisionCircles(const QVector<QPointF>& centers, const QVector<float>& radii) {
    m_game.setExternalCollisionCircles(centers, radii);
}

void FakeServer::setExternalHitPlayers(const QVector<quint32>& playerIds, const QVector<QPointF>& centers, const QVector<float>& radii) {
    m_game.setExternalHitPlayers(playerIds, centers, radii);
}
