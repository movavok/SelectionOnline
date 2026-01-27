#include "tilecollision.h"

const TileCollision &tileCollision(Tile::TileType type) {
    static const TileCollision empty { false, false };
    static const TileCollision board { true, true };
    static const TileCollision brickStrong { true, true };
    static const TileCollision brickCracked { true, true };
    static const TileCollision grass { false, false, 0.8f };
    static const TileCollision water { false, false, 0.4f };
    static const TileCollision wall { true, true };

    switch (type) {
    case Tile::TileType::Board: return board;
    case Tile::TileType::BrickStrong: return brickStrong;
    case Tile::TileType::BrickCracked: return brickCracked;
    case Tile::TileType::Grass: return grass;
    case Tile::TileType::Water: return water;
    case Tile::TileType::Wall: return wall;
    default: return empty;
    }
}
