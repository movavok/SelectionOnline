#include "tilevisual.h"

const TileVisual& tileVisual(Tile::TileType type) {
    static const TileVisual empty { Qt::transparent };
    static const TileVisual board { QColor(181, 131, 90) };
    static const TileVisual brickStrong { QColor(141, 47, 47) };
    static const TileVisual brickCracked { QColor(192, 75, 75) };
    static const TileVisual grass { QColor(80, 160, 80) };
    static const TileVisual water { QColor(60, 120, 200) };
    static const TileVisual wall { Qt::darkGray };

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
