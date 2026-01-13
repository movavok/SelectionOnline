#ifndef GAME_H
#define GAME_H

#include <QList>
#include "../entities/player.h"

class Game
{
public:
    Game();

    void update(float);

    void setPlayerInput(MoveDirection, bool);

    Player* getPlayer() const;

private:
    Player* m_player = nullptr;
    QList<Entity*> m_entities;
};

#endif // GAME_H
