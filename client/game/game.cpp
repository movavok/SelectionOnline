#include "game.h"

Game::Game() {
    m_map.loadFromFile(":/maps/default.txt");

    int mapWidth = m_map.getTileCountX() * Map::TILE_SIZE;
    int mapHeight = m_map.getTileCountY() * Map::TILE_SIZE;
    m_worldBounds = QRectF(-mapWidth / 2, -mapHeight / 2, mapWidth, mapHeight);

    m_player = new Player(QPointF(0, 0));
    m_player->setWeapon(WeaponManager::create("Katana"));
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

void Game::performWeaponHit(const Weapon& weapon, const QPointF& dir) {
    QPainterPath shape = weapon.indicatorShape(*m_player);

    const double angleDeg = std::atan2(dir.y(), dir.x()) * 180.0 / M_PI;

    QTransform rot;
    rot.rotate(angleDeg);
    QPainterPath worldShape = rot.map(shape);
    worldShape.translate(m_player->getPosition().x(), m_player->getPosition().y());

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

    const Weapon* weapon = m_player->getWeapon();
    if (!weapon) return;

    performWeaponHit(*weapon, attackDir);
    m_player->onAttackPerformed();
}

void Game::update(float deltaTime) {
    for (Entity* &entity : m_entities) {
        if (!entity->isAlive()) continue;

        if (Player* player = dynamic_cast<Player*>(entity)) {
            QPointF nextPos = player->getPosition() + player->moveDistance(deltaTime);
            if (canMove(player, nextPos)) player->setPosition(nextPos);
            player->update(deltaTime);
            processPlayerAttack();
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
