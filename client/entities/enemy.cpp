#include "enemy.h"

Enemy::Enemy(const QPointF &startPos)
    : Entity(startPos, 15.0f)
{
    m_hp = m_maxHp = 400;
}

void Enemy::takeDamage(int damage) {
    if (damage <= 0) { return; }
    if (damage >= m_hp) {
        m_hp = 0;
        return;
    }

    m_hp -= damage;
}
