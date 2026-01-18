#include "pickupitem.h"

PickupItem::PickupItem(const QPointF& startPos, Tile::TileType type)
    : m_position(startPos), m_type(type) {}

QPointF PickupItem::getPosition() const { return m_position; }

Tile::TileType PickupItem::getType() const { return m_type; }

