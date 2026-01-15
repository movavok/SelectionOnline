#include "tilevisual.h"

const TileVisual& tileVisual(Tile::TileType type) {
    static const TileVisual wall { Qt::darkGray };
    static const TileVisual empty { Qt::transparent };

    switch (type) {
    case Tile::TileType::Wall: return wall;
    default: return empty;
    }
}
