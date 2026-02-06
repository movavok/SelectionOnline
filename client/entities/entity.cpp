#include "entity.h"

Entity::Entity(const QPointF& pos, float radius)
    : m_position(pos), m_radius(radius) {}

float Entity::getRadius() const { return m_radius; }

void Entity::setPosition(const QPointF& pos) { m_position = pos; }
QPointF Entity::getPosition() const { return m_position; }
void Entity::setPrevPosition(const QPointF& pos) { m_prevPosition = pos; }
QPointF Entity::getPrevPosition() const { return m_prevPosition; }

void Entity::setImpulse(const QPointF& impulse) { m_impulse = impulse; }
void Entity::addImpulse(const QPointF& impulse) { m_impulse += impulse; }
const QPointF& Entity::getImpulse() const { return m_impulse; }

unsigned short Entity::getCurrentHp() const { return m_hp; }
unsigned short Entity::getMaxHp() const { return m_maxHp; }

void Entity::takeDamage(int damage) {
    if (damage <= 0) { return; }
    if (damage >= m_hp) { m_hp = 0; return; }

    m_hp -= damage;
}

bool Entity::isAlive() const { return m_hp > 0; }
