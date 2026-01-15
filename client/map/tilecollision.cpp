#include "tilecollision.h"

const TileCollision &tileCollision(Tile::TileType type) {
    static const TileCollision wall { true, true };
    static const TileCollision empty { false, false };

    switch (type) {
    case Tile::TileType::Wall: return wall;
    default: return empty;
    }
}
