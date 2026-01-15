#ifndef TILECOLLISION_H
#define TILECOLLISION_H

#include "tile.h"

struct TileCollision {
    bool personSolid;
    bool projectileSolid;
};

const TileCollision& tileCollision(Tile::TileType type);

#endif // TILECOLLISION_H
