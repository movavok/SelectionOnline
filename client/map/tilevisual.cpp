#include "tilevisual.h"

const TileVisual& tileVisual(Tile::TileType type) {
    static const TileVisual empty { QPixmap() };
    static const TileVisual board { QPixmap(":/tiles/board.png") };
    static const TileVisual brickStrong { QPixmap(":/tiles/brick_strong.png") };
    static const TileVisual brickCracked { QPixmap(":/tiles/brick_cracked.png") };
    static const TileVisual grass { QPixmap(":/tiles/grass.png") };
    static const TileVisual water { QPixmap(":/tiles/water.png") };
    static const TileVisual wall { QPixmap(":/tiles/wall.png") };

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
