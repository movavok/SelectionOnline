#include "tilevisual.h"

const TileVisual& tileVisual(Tile::TileType type) {
    static const TileVisual board { QPixmap(":/tiles/board.png") };
    static const TileVisual brickStrong { QPixmap(":/tiles/brick_strong.png") };
    static const TileVisual brickCracked { QPixmap(":/tiles/brick_cracked.png") };
    static const TileVisual grass { QPixmap(":/tiles/grass.png") };
    static const TileVisual water { QPixmap(":/tiles/water.png") };
    static const TileVisual wall { QPixmap(":/tiles/wall.png") };
    static const TileVisual empty { QPixmap() };

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

const TileVisual& emptyVisual(int variant) {
    static const TileVisual empty1 { QPixmap(":/tiles/empty_1.png") };
    static const TileVisual empty2 { QPixmap(":/tiles/empty_2.png") };
    static const TileVisual empty3 { QPixmap(":/tiles/empty_3.png") };

    switch (variant) {
    case 0: return empty1;
    case 1: return empty2;
    default: return empty3;
    }
}
