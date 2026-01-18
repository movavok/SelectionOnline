#ifndef PICKUPITEM_H
#define PICKUPITEM_H

#include <QPointF>

#include "tile.h"

class PickupItem
{
public:
    PickupItem(const QPointF& startPos, Tile::TileType type);

    QPointF getPosition() const;
    Tile::TileType getType() const;

private:
    QPointF m_position;
    Tile::TileType m_type;
};

#endif // PICKUPITEM_H
