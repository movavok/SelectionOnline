#include "game.h"

Game::Game(QObject* parent)
    : QObject(parent) {
    m_map.loadFromFile(":/maps/default.txt");
    m_map.setWorldBounds(m_map.getTileCountX() * Map::TILE_SIZE, m_map.getTileCountY() * Map::TILE_SIZE);
    m_worldBounds = m_map.getWorldBounds();

    initPlayer(pickPlayerSpawn());
}

static QPoint clampTileToMap(int tileX, int tileY, int tileCountX, int tileCountY) {
    return QPoint(std::clamp(tileX, 0, tileCountX - 1), std::clamp(tileY, 0, tileCountY - 1));
}

static QVector<QPoint> buildSpawnTiles(int tileCountX, int tileCountY) {
    const int maxTileX = tileCountX - 1;
    const int maxTileY = tileCountY - 1;

    const int innerMinX = std::min(2, maxTileX);
    const int innerMinY = std::min(2, maxTileY);
    const int innerMaxX = std::max(0, maxTileX - 2);
    const int innerMaxY = std::max(0, maxTileY - 2);

    const int midX = tileCountX / 2;
    const int midY = tileCountY / 2;
    return {
        clampTileToMap(innerMinX, innerMinY, tileCountX, tileCountY), clampTileToMap(innerMaxX, innerMinY, tileCountX, tileCountY),
        clampTileToMap(innerMinX, innerMaxY, tileCountX, tileCountY), clampTileToMap(innerMaxX, innerMaxY, tileCountX, tileCountY),
        clampTileToMap(midX,      innerMinY, tileCountX, tileCountY), clampTileToMap(midX,      innerMaxY, tileCountX, tileCountY),
        clampTileToMap(innerMinX, midY,      tileCountX, tileCountY), clampTileToMap(innerMaxX, midY,      tileCountX, tileCountY),
        clampTileToMap(midX - 2,  midY,      tileCountX, tileCountY), clampTileToMap(midX + 2,  midY,      tileCountX, tileCountY),
    };
}

static QVector<int> buildShuffledIndices(int count) {
    QVector<int> indices;
    indices.resize(std::max(0, count));
    for (int index = 0; index < indices.size(); ++index) indices[index] = index;

    std::shuffle(indices.begin(), indices.end(), *QRandomGenerator::global());
    return indices;
}

static bool isFreeSpawn(const Map& map, const QPointF& pos, float radius) {
    const QRectF bounds(pos.x() - radius, pos.y() - radius, radius * 2.0f, radius * 2.0f);
    return !map.intersectsSolid(bounds, Map::CollisionActor::Person);
}

QPointF Game::pickPlayerSpawn() const {
    const int tileCountX = int(m_map.getTileCountX());
    const int tileCountY = int(m_map.getTileCountY());
    const QVector<QPoint> spawnTiles = buildSpawnTiles(tileCountX, tileCountY);
    const QVector<int> spawnOrder = buildShuffledIndices(spawnTiles.size());

    const float playerRadius = 15.0f;
    for (int index : spawnOrder) {
        const QPointF pos = m_map.tileToWorld(spawnTiles[index]);
        if (isFreeSpawn(m_map, pos, playerRadius))
            return pos;
    }
    return QPointF(0, 0);
}

void Game::initPlayer(const QPointF& playerSpawn) {
    m_player = new Player(playerSpawn);
    m_player->getInventory().setActiveSlot(0);
    m_player->getInventory().addWeapon(WeaponManager::create("Katana"));
    m_entities.push_back(m_player);
}

bool circlesIntersect(const Entity* first, const QPointF& newPos, const Entity* second) {
    return QLineF(newPos, second->getPosition()).length() < (first->getRadius() + second->getRadius());
}

bool rectCircleIntersect(const QRectF& rect, const QPointF& circleCenter, float radius) {
    float closestX = std::clamp(circleCenter.x(), rect.left(), rect.right());
    float closestY = std::clamp(circleCenter.y(), rect.top(), rect.bottom());

    float deltaX = circleCenter.x() - closestX;
    float deltaY = circleCenter.y() - closestY;

    return (deltaX * deltaX + deltaY * deltaY) <= (radius * radius);
}

bool Game::canPlaceTile(const QPoint& tilePos) const {
    const Tile& tile = m_map.tileAt(tilePos.x(), tilePos.y());

    if (tile.getType() != Tile::TileType::Empty) return false;

    QPointF center = m_map.tileToWorld(tilePos);
    float dist = QLineF(center, m_player->getPosition()).length();
    if (dist > Map::TILE_SIZE * 5) return false;

    QRectF tileRect(center - QPointF(Map::TILE_SIZE / 2, Map::TILE_SIZE / 2),
                    QSizeF(Map::TILE_SIZE, Map::TILE_SIZE));

    for (Entity* entity : m_entities) {
        if (!entity->isAlive()) continue;
        if (rectCircleIntersect(tileRect, entity->getPosition(), entity->getRadius()))
            return false;
    }

    const int extCount = std::min(m_externalCollisionCenters.size(), m_externalCollisionRadii.size());
    for (int index = 0; index < extCount; ++index)
        if (rectCircleIntersect(tileRect, m_externalCollisionCenters[index], m_externalCollisionRadii[index]))
            return false;

    return true;
}

void Game::setExternalCollisionCircles(const QVector<QPointF>& centers, const QVector<float>& radii) {
    m_externalCollisionCenters = centers;
    m_externalCollisionRadii = radii;
}

void Game::setExternalHitPlayers(const QVector<quint32>& playerIds, const QVector<QPointF>& centers, const QVector<float>& radii) {
    m_externalHitPlayerIds = playerIds;
    m_externalHitCenters = centers;
    m_externalHitRadii = radii;
}

bool Game::tryPlaceTile(const QPoint& tilePos) {
    Inventory& inventory = m_player->getInventory();
    if (!inventory.isActiveResource()) return false;

    Tile::TileType type = inventory.getActiveResourceType();

    if (!canPlaceTile(tilePos)) return false;
    if (!inventory.spendResource(type, 1)) return false;

    m_map.tileAt(tilePos.x(), tilePos.y()).setType(type);
    emit tileChanged(tilePos.x(), tilePos.y());
    return true;
}

bool Game::collidesWithSolidTiles(const QPointF& newPos, float radius) const {
    const QRectF rectBounds(newPos - QPointF(radius, radius), QSizeF(radius * 2, radius * 2));

    QVector<QPoint> tiles;
    m_map.tilesInRect(rectBounds, tiles);
    for (const QPoint& tilePos : tiles) {
        const TileCollision& collision = tileCollision(m_map.tileAt(tilePos.x(), tilePos.y()).getType());
        if (!collision.personSolid) continue;

        const QPointF center = m_map.tileToWorld(tilePos);
        const QRectF tileRect(center - QPointF(Map::TILE_SIZE / 2.0, Map::TILE_SIZE / 2.0),
                              QSizeF(Map::TILE_SIZE, Map::TILE_SIZE));

        if (rectCircleIntersect(tileRect, newPos, radius))
            return true;
    }
    return false;
}

bool Game::collidesWithEntities(const Entity* entity, const QPointF& newPos) const {
    for (Entity* other : m_entities) {
        if (other == entity || !other->isAlive()) continue;
        if (circlesIntersect(entity, newPos, other))
            return true;
    }
    return false;
}

bool Game::collidesWithExternalCircles(const Entity* entity, const QPointF& newPos) const {
    const int extCount = std::min(m_externalCollisionCenters.size(), m_externalCollisionRadii.size());
    for (int index = 0; index < extCount; ++index) {
        const QPointF& center = m_externalCollisionCenters[index];
        const float radius = m_externalCollisionRadii[index];
        if (QLineF(newPos, center).length() < (entity->getRadius() + radius))
            return true;
    }
    return false;
}

bool Game::canMove(const Entity* entity, const QPointF& newPos) const {
    const float radius = entity->getRadius();
    const QRectF rectBounds(newPos - QPointF(radius, radius), QSizeF(radius * 2, radius * 2));

    if (!m_worldBounds.contains(rectBounds)) return false;
    if (collidesWithSolidTiles(newPos, radius)) return false;
    if (collidesWithEntities(entity, newPos)) return false;
    if (collidesWithExternalCircles(entity, newPos)) return false;
    return true;
}

void Game::spawnPickupAtTile(const QPoint& pos, Tile::TileType type) {
    m_pickups.push_back(new PickupItem(m_map.tileToWorld(pos), type));
}

void Game::maybeSpawnPickupForBrokenTile(const QPoint& tilePos, Tile::TileType oldType) {
    Tile::TileType dropType = Tile::TileType::Empty;
    switch (oldType) {
    case Tile::TileType::BrickCracked: dropType = Tile::TileType::BrickStrong; break;
    case Tile::TileType::Board: dropType = Tile::TileType::Board; break;
    default: break;
    }

    if (dropType != Tile::TileType::Empty)
        spawnPickupAtTile(tilePos, dropType);
}

QPainterPath Game::makeCirclePath(const QPointF& center, float radius) const {
    QPainterPath circle;
    circle.addEllipse(center, radius, radius);
    return circle;
}

static QPainterPath buildWeaponAttackPath(const Weapon& weapon, const Player& player, const QPointF& mouseScene) {
    const QPointF playerPos = player.getPosition();
    const QPointF direction = mouseScene - playerPos;
    const double angleDeg = std::atan2(direction.y(), direction.x()) * 180.0 / M_PI;

    QTransform rotation;
    rotation.rotate(angleDeg);

    QPainterPath attackPath = rotation.map(weapon.indicatorShape(player));
    attackPath.translate(playerPos);
    return attackPath;
}


QPainterPath Game::getPlayerAttackShape(const QPointF& mouseScene) const {
    const Weapon* weapon = m_player->getInventory().getActiveWeapon();
    if (!weapon)
        return {};

    const QPointF playerPos = m_player->getPosition();
    const QPainterPath attackPath = buildWeaponAttackPath(*weapon, *m_player, mouseScene);
    return AttackFxUtils::clipAttackPathToObstacles(attackPath, playerPos, m_map);
}

void Game::tryBreakTileAt(const QPoint& tilePos, const QPainterPath& hitShape) {
    Tile& tile = m_map.tileAt(tilePos.x(), tilePos.y());
    const Tile::TileType oldType = tile.getType();

    const QPointF center = m_map.tileToWorld(tilePos);
    const QRectF tileRect(center - QPointF(Map::TILE_SIZE / 2, Map::TILE_SIZE / 2),
                          QSizeF(Map::TILE_SIZE, Map::TILE_SIZE));

    if (!hitShape.intersects(tileRect))
        return;
    if (!tile.applyHit())
        return;

    Tile::TileType dropType = Tile::TileType::Empty;
    switch (oldType) {
    case Tile::TileType::BrickCracked: dropType = Tile::TileType::BrickStrong; break;
    case Tile::TileType::Board: dropType = Tile::TileType::Board; break;
    default: break;
    }

    if (dropType != Tile::TileType::Empty)
        spawnPickupAtTile(tilePos, dropType);

    emit tileChanged(tilePos.x(), tilePos.y());
}

void Game::tryBreakTiles(const QPainterPath& hitShape) {
    const QRectF bounds = hitShape.boundingRect();

    const QPoint minTile = m_map.worldToTile(bounds.topLeft());
    const QPoint maxTile = m_map.worldToTile(bounds.bottomRight());

    for (int tileY = minTile.y(); tileY <= maxTile.y(); ++tileY) {
        for (int tileX = minTile.x(); tileX <= maxTile.x(); ++tileX) {
            tryBreakTileAt(QPoint(tileX, tileY), hitShape);
        }
    }
}

static QPointF normalizedDirection(const QPointF& from, const QPointF& to) {
    QPointF direction = to - from;
    const double len = std::hypot(direction.x(), direction.y());
    if (len <= 0.0) return QPointF(0, 0);
    return direction / len;
}

static int applyDamageAndGetApplied(Entity* target, int baseDamage) {
    const unsigned short hpBefore = target->getCurrentHp();
    target->takeDamage(baseDamage);
    const unsigned short hpAfter = target->getCurrentHp();
    return std::max(0, int(hpBefore) - int(hpAfter));
}

void Game::performWeaponHitOnEntities(const Weapon& weapon, const QPainterPath& attackShape) {
    for (Entity* target : m_entities) {
        if (target == m_player || !target->isAlive())
            continue;

        const QPainterPath targetCircle = makeCirclePath(target->getPosition(), target->getRadius());
        if (!attackShape.intersects(targetCircle))
            continue;

        const int baseDamage = weapon.getDamage();
        const int appliedDamage = applyDamageAndGetApplied(target, baseDamage);
        if (appliedDamage <= 0)
            continue;

        const QPointF hitDir = normalizedDirection(m_player->getPosition(), target->getPosition());

        HitInfo hit;
        hit.attacker = m_player;
        hit.target = target;
        hit.damage = appliedDamage;
        hit.direction = hitDir;
        emit enemyHit(hit);

        const double knockbackStrength = std::clamp(appliedDamage * 2.0, 0.0, 500.0);
        target->addImpulse(hitDir * knockbackStrength);
    }
}

void Game::performWeaponHitOnExternalPlayers(const Weapon& weapon, const QPainterPath& attackShape) {
    const int extCount = std::min({m_externalHitPlayerIds.size(), m_externalHitCenters.size(), m_externalHitRadii.size()});
    if (extCount <= 0) return;

    const quint16 baseDamage = weapon.getDamage();
    for (int index = 0; index < extCount; ++index) {
        const quint32 targetId = m_externalHitPlayerIds[index];
        if (targetId == 0) continue;

        const QPainterPath targetCircle = makeCirclePath(m_externalHitCenters[index], m_externalHitRadii[index]);
        if (!attackShape.intersects(targetCircle))
            continue;

        emit externalPlayerHit(targetId, baseDamage);
    }
}

void Game::performWeaponHit(const Weapon& weapon, const QPointF& dir, const QPainterPath& attackShape) {
    Q_UNUSED(dir);
    if (attackShape.isEmpty())
        return;

    performWeaponHitOnEntities(weapon, attackShape);
    performWeaponHitOnExternalPlayers(weapon, attackShape);
}

void Game::processPlayerAttack() {
    if (!m_player) return;

    QPointF attackDir;
    if (!m_player->consumeAttackRequest(attackDir)) return;
    if (!m_player->canAttack()) return;

    const Weapon* weapon = m_player->getInventory().getActiveWeapon();
    if (!weapon) return;

    QPainterPath attackShape = getPlayerAttackShape(m_player->getPosition() + attackDir);
    if (!attackShape.isEmpty()) {
        performWeaponHit(*weapon, attackDir, attackShape);
        tryBreakTiles(attackShape);
    }

    emit localPlayerAttackPerformed(float(attackDir.x()), float(attackDir.y()));

    m_player->onAttackPerformed();
}

void Game::applyPickup(PickupItem* pickup) {
    switch (pickup->getType()) {
    case Tile::TileType::BrickStrong:
    case Tile::TileType::BrickCracked:
    case Tile::TileType::Board:
        m_player->getInventory().addResource(pickup->getType(), 1); break;
    default: break;
    }
}

void Game::checkPickupCollisions() {
    if (!m_player) return;

    for (int index = m_pickups.size() - 1; index >= 0; --index) {
        PickupItem* pickup = m_pickups[index];

        float dist = QLineF(m_player->getPosition(), pickup->getPosition()).length();
        if (dist <= m_player->getRadius() + Map::TILE_SIZE / 2) {
            applyPickup(pickup);
            const QPoint tilePos = m_map.worldToTile(pickup->getPosition());
            emit pickupCollected(qint16(tilePos.x()), qint16(tilePos.y()));
            removePickupAtTile(tilePos);
            return;
        }
    }
}

bool Game::removePickupAtTile(const QPoint& tilePos) {
    bool removedAny = false;
    for (int index = m_pickups.size() - 1; index >= 0; --index) {
        PickupItem* pickup = m_pickups[index];
        if (!pickup) continue;
        if (m_map.worldToTile(pickup->getPosition()) != tilePos)
            continue;

        delete pickup;
        m_pickups.removeAt(index);
        removedAny = true;
    }
    return removedAny;
}

void Game::updateNonPlayerEntity(Entity* entity, float deltaTime) {
    const QPointF delta = entity->getImpulse() * deltaTime;
    const QPointF currPos = entity->getPosition();
    const QPointF nextPos = currPos + delta;

    if (canMove(entity, nextPos)) {
        entity->setPrevPosition(currPos);
        entity->setPosition(nextPos);
        return;
    }

    entity->setImpulse(QPointF(0, 0));
}

void Game::updatePlayer(Player* player, float deltaTime) {
    QPointF delta = player->moveDistance(deltaTime);

    const QPoint tilePos = m_map.worldToTile(player->getPosition());
    delta *= tileCollision(m_map.tileAt(tilePos.x(), tilePos.y()).getType()).moveViscosity;
    delta += player->getImpulse() * deltaTime;

    const QPointF currPos = player->getPosition();
    const QPointF nextPos = currPos + delta;

    if (canMove(player, nextPos)) {
        player->setPrevPosition(currPos);
        player->setPosition(nextPos);
    } else {
        player->setImpulse(QPointF(0, 0));
    }

    processPlayerAttack();
    checkPickupCollisions();
}

void Game::updateEntity(Entity* entity, float deltaTime) {
    if (!entity->isAlive()) return;

    if (Player* player = dynamic_cast<Player*>(entity)) {
        updatePlayer(player, deltaTime);
    } else {
        updateNonPlayerEntity(entity, deltaTime);
    }

    entity->update(deltaTime);
    entity->setImpulse(entity->getImpulse() * 0.9);
}

void Game::update(float deltaTime) {
    for (Entity* entity : m_entities)
        updateEntity(entity, deltaTime);
}

void Game::setPlayerInput(MoveDirection key, bool pressed) {
    if (m_player) m_player->setInput(key, pressed);
}

Player* Game::getPlayer() const { return m_player; }

const QList<Entity*>& Game::getEntities() const { return m_entities; }

Map& Game::getMap() { return m_map; }

const Map& Game::getMap() const { return m_map; }

QRectF Game::getWorldBounds() const { return m_worldBounds; }

const QList<PickupItem*>& Game::getPickups() const { return m_pickups; }
