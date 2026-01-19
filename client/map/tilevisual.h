#ifndef TILEVISUAL_H
#define TILEVISUAL_H

#include <QPixmap>

#include "tile.h"

struct TileVisual { QPixmap sprite; };

const TileVisual& tileVisual(Tile::TileType);

#endif // TILEVISUAL_H
