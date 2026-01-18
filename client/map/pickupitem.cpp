#include "pickupitem.h"

PickupItem::Pickedup(const QPointF& startPos, Tile::TileType type)
    : m_position(startPos), m_type(type) {}

QPointF PickupItem::getPosition() const { return m_position; }

Tile::TileType PickupItem::getType() const { return m_type; }

