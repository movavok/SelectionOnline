#include "enemy.h"

Enemy::Enemy(const QPointF &startPos)
    : Entity(startPos, 15.0f)
{
    m_hp = m_maxHp = 400;
}
