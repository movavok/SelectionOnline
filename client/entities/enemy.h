#ifndef ENEMY_H
#define ENEMY_H

#include "entity.h"

class Enemy : public Entity
{
public:
    Enemy(const QPointF& startPos);

    void update(float) override {};

private:
    float m_speed = 100.0f;
};
#endif // ENEMY_H
