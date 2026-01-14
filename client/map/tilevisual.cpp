#include "tilevisual.h"

const TileVisual& tileVisual(Tile::TileType type) {
    static const TileVisual wall { Qt::darkGray, true, true };
    static const TileVisual empty { Qt::transparent, false, false };

    switch (type) {
    case Tile::TileType::Wall: return wall;
    default: return empty;
    }
}
