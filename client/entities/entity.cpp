#include "entity.h"

Entity::Entity(const QPointF& pos) : m_position(pos) {}

void Entity::setPosition(const QPointF& pos) { m_position = pos; }
QPointF Entity::getPosition() const { return m_position; }

unsigned short Entity::getWidth() const { return m_width; }
unsigned short Entity::getHeight() const { return m_height; }

bool Entity::isAlive() const { return m_alive; }
void Entity::destroy() { m_alive = false; }
