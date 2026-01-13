#include "game.h"

Game::Game()
{
    m_player = new Player(QPointF(0, 0));
    m_entities.push_back(m_player);
}

void Game::update(float deltaTime)
{
    for (Entity* &entity : m_entities)
        if (entity->isAlive()) entity->update(deltaTime);
}

void Game::setPlayerInput(MoveDirection key, bool pressed) {
    if (m_player) m_player->setInput(key, pressed);
}

Player* Game::getPlayer() const { return m_player; }
