#include "tile.h"

Tile::Tile(TileType type) : m_type(type) {}

Tile::TileType Tile::getType() const { return m_type; }

bool Tile::applyHit() {
    switch (m_type) {
    case TileType::Board: m_type = TileType::Empty; return true;
    case TileType::BrickCracked: m_type = TileType::Empty; return true;
    case TileType::BrickStrong: m_type = TileType::BrickCracked; return true;
    default: return false;
    }
}
