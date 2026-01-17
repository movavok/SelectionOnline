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

void Game::update(float deltaTime) {
    for (Entity* &entity : m_entities) {
        if (!entity->isAlive()) continue;

        if (Player* player = dynamic_cast<Player*>(entity)) {
            QPointF nextPos = player->getPosition() + player->moveDistance(deltaTime);
            if (canMove(player, nextPos)) player->setPosition(nextPos);
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
