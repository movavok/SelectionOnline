#include "entity.h"

Entity::Entity(const QPointF& pos, float radius)
    : m_position(pos), m_radius(radius) {}

float Entity::getRadius() const { return m_radius; }

void Entity::setPosition(const QPointF& pos) { m_position = pos; }
QPointF Entity::getPosition() const { return m_position; }

unsigned short Entity::getCurrentHp() const { return m_hp; }
unsigned short Entity::getMaxHp() const { return m_maxHp; }

bool Entity::isAlive() const { return m_alive; }
void Entity::destroy() { m_alive = false; }
