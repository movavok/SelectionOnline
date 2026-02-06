#ifndef TILEVISUAL_H
#define TILEVISUAL_H

#include <QPixmap>
#include <cstdlib>
#include <ctime>

#include "tile.h"

struct TileVisual {
    QPixmap sprite;
};

const TileVisual& tileVisual(Tile::TileType);
const TileVisual& emptyVisual(int variant);

#endif // TILEVISUAL_H
