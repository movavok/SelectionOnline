#ifndef TILEVISUAL_H
#define TILEVISUAL_H

#include <QColor>

#include "tile.h"

struct TileVisual {
    QColor color;
    bool personSolid;
    bool projectileSolid;
};

const TileVisual& tileVisual(Tile::TileType);

#endif // TILEVISUAL_H
