#include "game.h"

Game::Game()
{
    m_map.loadFromFile(":/maps/default.txt");

    int mapWidth = m_map.getTileCountX() * Map::TILE_SIZE;
    int mapHeight = m_map.getTileCountY() * Map::TILE_SIZE;
    m_worldBounds = QRectF(-mapWidth / 2, -mapHeight / 2, mapWidth, mapHeight);

    m_player = new Player(QPointF(0, 0));
    m_entities.push_back(m_player);
}

bool Game::canMove(const Entity* entity, const QPointF& newPos) const
{
    QRectF newBounds = entity->bounds();
    newBounds.moveTo(newPos);

    if (!m_worldBounds.contains(newBounds)) return false;
    if (m_map.intersectsAnyTiles(newBounds, { Tile::TileType::Wall }))
        return false;

    for (Entity* other : m_entities) {
        if (other == entity || !other->isAlive()) continue;
        if (newBounds.intersects(other->bounds())) return false;
    }

    return true;
}

void Game::update(float deltaTime)
{
    for (Entity* &entity : m_entities) {
        if (!entity->isAlive()) continue;

        entity->update(deltaTime);

        if (Player* player = dynamic_cast<Player*>(entity)) {
            QPointF nextPos = player->getPosition() + player->moveDistance(deltaTime);
            if (canMove(player, nextPos)) player->setPosition(nextPos);
        }
    }
}

void Game::setPlayerInput(MoveDirection key, bool pressed) {
    if (m_player) m_player->setInput(key, pressed);
}

Player* Game::getPlayer() const { return m_player; }

const Map& Game::getMap() const { return m_map; }

QRectF Game::getWorldBounds() const { return m_worldBounds; }
