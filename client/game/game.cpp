#include "game.h"

Game::Game(QObject* parent) : QObject(parent) {
    m_map.loadFromFile(":/maps/default.txt");

    int mapWidth = m_map.getTileCountX() * Map::TILE_SIZE;
    int mapHeight = m_map.getTileCountY() * Map::TILE_SIZE;
    m_worldBounds = QRectF(-mapWidth / 2, -mapHeight / 2, mapWidth, mapHeight);

    m_player = new Player(QPointF(0, 0));
    m_player->getInventory().addWeapon(WeaponManager::create("Katana"));
    m_entities.push_back(m_player);

    Enemy* enemy = new Enemy(QPointF(100, 0));
    m_entities.push_back(enemy);
}

bool circlesIntersect(const Entity* first, const QPointF& newPos, const Entity* second) {
    return QLineF(newPos, second->getPosition()).length() < (first->getRadius() + second->getRadius());
}

bool Game::canMove(const Entity* entity, const QPointF& newPos) const {
    const float radius = entity->getRadius();
    QRectF rectBounds = QRectF(newPos - QPointF(radius, radius), QSizeF(radius * 2, radius * 2));

    if (!m_worldBounds.contains(rectBounds))
        return false;
    if (m_map.intersectsSolid(rectBounds, Map::CollisionActor::Person))
        return false;

    for (Entity* other : m_entities) {
        if (other == entity || !other->isAlive()) continue;
        if (circlesIntersect(entity, newPos, other)) return false;
    }

    return true;
}

void Game::spawnPickupAtTile(const QPointF& pos, Tile::TileType type) {
    m_pickups.emplace_back(Map::tileToWorld(pos, m_worldBounds), type);
}

void Game::tryBreakTiles(const QPainterPath& hitShape) {
    QRectF bounds = hitShape.boundingRect();

    int minX = std::floor((bounds.left() + m_worldBounds.width() / 2) / Map::TILE_SIZE);
    int maxX = std::floor((bounds.right() + m_worldBounds.width() / 2) / Map::TILE_SIZE);
    int minY = std::floor((bounds.top() + m_worldBounds.height() / 2) / Map::TILE_SIZE);
    int maxY = std::floor((bounds.bottom()+ m_worldBounds.height() / 2) / Map::TILE_SIZE);

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            Tile& tile = m_map.tileAt(x, y);
            if (tile.applyHit()) {
                spawnPickupAtTile(QPointF(x, y), tile.getType());
                emit tileChanged(x, y);
            }
        }
    }
}

void Game::performWeaponHit(const Weapon& weapon, const QPointF& dir) {
    QPainterPath shape = weapon.indicatorShape(*m_player);

    const double angleDeg = std::atan2(dir.y(), dir.x()) * 180.0 / M_PI;

    QTransform rot;
    rot.rotate(angleDeg);
    QPainterPath worldShape = rot.map(shape);
    worldShape.translate(m_player->getPosition().x(), m_player->getPosition().y());

    tryBreakTiles(worldShape);

    for (Entity* &entity : m_entities) {
        if (entity == m_player || !entity->isAlive()) continue;
        if (Enemy* enemy = dynamic_cast<Enemy*>(entity)) {
            QPainterPath enemyPath;
            enemyPath.addEllipse(enemy->getPosition(), enemy->getRadius(), enemy->getRadius());

            if (worldShape.intersects(enemyPath))
                enemy->takeDamage(weapon.getDamage());
        }
    }
}

void Game::processPlayerAttack() {
    if (!m_player) return;

    QPointF attackDir;
    if (!m_player->consumeAttackRequest(attackDir)) return;
    if (!m_player->canAttack()) return;

    const Weapon* weapon = m_player->getInventory().getActiveWeapon();
    if (!weapon) return;

    performWeaponHit(*weapon, attackDir);
    m_player->onAttackPerformed();
}

void Game::applyPickup(PickupItem& pickup) {
    switch (pickup.getType()) {
    case Tile::TileType::BrickCracked:
    case Tile::TileType::Board: m_player->getInventory().addResource(pickup.getType(), 1); break;
    default: break;
    }
}

void Game::checkPickupCollisions() {
    if (!m_player) return;

    for (int index = m_pickups.size() - 1; index >= 0; --index) {
        PickupItem& pickup = m_pickups[index];

        float dist = QLineF(m_player->getPosition(), pickup.getPosition()).length();
        if (dist <= m_player->getRadius() + Map::TILE_SIZE / 2) {
            applyPickup(pickup);
            m_pickups.removeAt(index);
        }
    }
}

void Game::update(float deltaTime) {
    for (Entity* &entity : m_entities) {
        if (!entity->isAlive()) continue;

        if (Player* player = dynamic_cast<Player*>(entity)) {
            QPointF nextPos = player->getPosition() + player->moveDistance(deltaTime);
            if (canMove(player, nextPos)) player->setPosition(nextPos);
            player->update(deltaTime);
            processPlayerAttack();
            checkPickupCollisions();
        } else entity->update(deltaTime);
    }
}

void Game::setPlayerInput(MoveDirection key, bool pressed) {
    if (m_player) m_player->setInput(key, pressed);
}

Player* Game::getPlayer() const { return m_player; }

const QList<Entity*>& Game::getEntities() const { return m_entities; }

const Map& Game::getMap() const { return m_map; }

QRectF Game::getWorldBounds() const { return m_worldBounds; }
