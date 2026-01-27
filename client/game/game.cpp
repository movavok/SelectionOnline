#include "game.h"

Game::Game(QObject* parent) : QObject(parent) {
    m_map.loadFromFile(":/maps/default.txt");
    m_map.setWorldBounds(m_map.getTileCountX() * Map::TILE_SIZE, m_map.getTileCountY() * Map::TILE_SIZE);
    m_worldBounds = m_map.getWorldBounds();

    m_player = new Player(QPointF(0, 0));
    m_player->getInventory().setActiveSlot(0);
    m_player->getInventory().addWeapon(WeaponManager::create("Katana"));
    m_entities.push_back(m_player);

    Enemy* enemy = new Enemy(QPointF(100, 0));
    m_entities.push_back(enemy);
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

    return true;
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

bool Game::canMove(const Entity* entity, const QPointF& newPos) const {
    const float radius = entity->getRadius();
    QRectF rectBounds = QRectF(newPos - QPointF(radius, radius), QSizeF(radius * 2, radius * 2));

    if (!m_worldBounds.contains(rectBounds))
        return false;

    QVector<QPoint> tiles;
    m_map.tilesInRect(rectBounds, tiles);
    for (const QPoint& tilePos : tiles) {
        const TileCollision& collision = tileCollision(m_map.tileAt(tilePos.x(), tilePos.y()).getType());
        if (!collision.personSolid) continue;

        const QPointF center = m_map.tileToWorld(tilePos);
        const QRectF tileRect(center - QPointF(Map::TILE_SIZE / 2.0, Map::TILE_SIZE / 2.0),
                              QSizeF(Map::TILE_SIZE, Map::TILE_SIZE));

        if (rectCircleIntersect(tileRect, newPos, radius)) return false;
    }

    for (Entity* other : m_entities) {
        if (other == entity || !other->isAlive()) continue;
        if (circlesIntersect(entity, newPos, other)) return false;
    }

    return true;
}

void Game::spawnPickupAtTile(const QPoint& pos, Tile::TileType type) {
    m_pickups.push_back(new PickupItem(m_map.tileToWorld(pos), type));
}

QPainterPath Game::makeCirclePath(const QPointF& center, float radius) const {
    QPainterPath circle;
    circle.addEllipse(center, radius, radius);
    return circle;
}

QPainterPath Game::getPlayerAttackShape(const QPointF& mouseScene) const {
    const Weapon* weapon = m_player->getInventory().getActiveWeapon();
    if (!weapon)
        return {};

    const QPointF playerPos = m_player->getPosition();
    const QPointF direction = mouseScene - playerPos;

    const double angleDeg =
        std::atan2(direction.y(), direction.x()) * 180.0 / M_PI;

    QTransform rotation;
    rotation.rotate(angleDeg);

    QPainterPath attackPath =
        rotation.map(weapon->indicatorShape(*m_player));

    attackPath.translate(playerPos);

    QPainterPath result;

    for (const QPolygonF& polygon : attackPath.toSubpathPolygons()) {
        if (polygon.size() < 3)
            continue;

        QPolygonF clippedPolygon;
        clippedPolygon.reserve(polygon.size());

        for (const QPointF& point : polygon) {
            clippedPolygon << AttackRaycast::clampToObstacle(
                playerPos, point, m_map
                );
        }

        if (clippedPolygon.size() < 3)
            continue;

        const QRectF bounds = clippedPolygon.boundingRect();
        if (bounds.width() < 0.5 && bounds.height() < 0.5)
            continue;

        result.addPolygon(clippedPolygon);
    }

    return result;
}

void Game::tryBreakTiles(const QPainterPath& hitShape) {
    QRectF bounds = hitShape.boundingRect();

    QPoint minTile = m_map.worldToTile(bounds.topLeft());
    QPoint maxTile = m_map.worldToTile(bounds.bottomRight());

    for (int y = minTile.y(); y <= maxTile.y(); ++y) {
        for (int x = minTile.x(); x <= maxTile.x(); ++x) {

            QPoint tilePos(x, y);
            Tile& tile = m_map.tileAt(x, y);
            Tile::TileType oldType = tile.getType();

            QPointF center = m_map.tileToWorld(tilePos);

            QRectF tileRect(center - QPointF(Map::TILE_SIZE / 2, Map::TILE_SIZE / 2),
                            QSizeF(Map::TILE_SIZE, Map::TILE_SIZE));

            if (hitShape.intersects(tileRect) && tile.applyHit()) {
                Tile::TileType dropType = Tile::TileType::Empty;
                switch (oldType) {
                case Tile::TileType::BrickCracked: dropType = Tile::TileType::BrickStrong; break;
                case Tile::TileType::Board: dropType = Tile::TileType::Board; break;
                default: break;
                }
                if (dropType != Tile::TileType::Empty)
                    spawnPickupAtTile(QPoint(x, y), dropType);

                emit tileChanged(x, y);
            }
        }
    }
}

void Game::performWeaponHit(const Weapon& weapon, const QPointF& dir, const QPainterPath& attackShape) {
    Q_UNUSED(dir);

    if (attackShape.isEmpty())
        return;

    for (Entity* enemy : m_entities) {
        if (enemy == m_player || !enemy->isAlive())
            continue;

        const QPainterPath enemyCircle = makeCirclePath(enemy->getPosition(), enemy->getRadius());
        if (!attackShape.intersects(enemyCircle))
            continue;

        const int baseDamage = weapon.getDamage();
        const unsigned short hpBefore = enemy->getCurrentHp();
        enemy->takeDamage(baseDamage);
        const unsigned short hpAfter = enemy->getCurrentHp();
        const int appliedDamage = int(hpBefore) - int(hpAfter);

        QPointF hitDir = enemy->getPosition() - m_player->getPosition();
        const double hitLen = std::hypot(hitDir.x(), hitDir.y());
        if (hitLen > 0.0)
            hitDir /= hitLen;

        HitInfo hit;
        hit.attacker = m_player;
        hit.target = enemy;
        hit.damage = appliedDamage;
        hit.direction = hitDir;
        emit enemyHit(hit);

        QPointF knockbackDir = hitDir;

        const double knockbackStrength = std::clamp(appliedDamage * 2.0, 0.0, 500.0);
        enemy->addImpulse(knockbackDir * knockbackStrength);
    }
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
            delete m_pickups[index];
            m_pickups.removeAt(index);
        }
    }
}

void Game::update(float deltaTime) {
    for (Entity* &entity : m_entities) {
        if (!entity->isAlive()) continue;

        QPointF delta;

        if (Player* player = dynamic_cast<Player*>(entity)) {
            delta = player->moveDistance(deltaTime);
            QPoint tilePos = m_map.worldToTile(player->getPosition());
            delta *= tileCollision(m_map.tileAt(tilePos.x(), tilePos.y()).getType()).moveViscosity;

            delta += player->getImpulse() * deltaTime;

            QPointF currPos = player->getPosition();
            QPointF nextPos = currPos + delta;

            if (canMove(player, nextPos)) {
                player->setPrevPosition(currPos);
                player->setPosition(nextPos);
            } else
                player->setImpulse(QPointF(0, 0));

            processPlayerAttack();
            checkPickupCollisions();

        } else {
            delta = entity->getImpulse() * deltaTime;
            QPointF currPos = entity->getPosition();
            QPointF nextPos = currPos + delta;

            if (canMove(entity, nextPos)) {
                entity->setPrevPosition(currPos);
                entity->setPosition(nextPos);
            } else
                entity->setImpulse(QPointF(0, 0));
        }

        entity->update(deltaTime);
        entity->setImpulse(entity->getImpulse() * 0.9);
    }
}

void Game::setPlayerInput(MoveDirection key, bool pressed) {
    if (m_player) m_player->setInput(key, pressed);
}

Player* Game::getPlayer() const { return m_player; }

const QList<Entity*>& Game::getEntities() const { return m_entities; }

const Map& Game::getMap() const { return m_map; }

QRectF Game::getWorldBounds() const { return m_worldBounds; }

const QList<PickupItem*>& Game::getPickups() const { return m_pickups; }
