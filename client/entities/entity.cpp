#include "entity.h"

Entity::Entity(const QPointF& pos) : m_position(pos) {}

void Entity::setPosition(const QPointF& pos) { m_position = pos; }
QPointF Entity::getPosition() const { return m_position; }

bool Entity::isAlive() const { return m_alive; }
void Entity::destroy() { m_alive = false; }
