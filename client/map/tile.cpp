#include "tile.h"

Tile::Tile(TileType type) : m_type(type) {}

void Tile::setType(TileType type) { m_type = type; }

Tile::TileType Tile::getType() const { return m_type; }

void Tile::setVariation(unsigned short variation) { m_variation = variation; }

unsigned short Tile::getVariation() const { return m_variation; }

bool Tile::applyHit() {
    switch (m_type) {
    case TileType::Board: m_type = TileType::Empty; return true;
    case TileType::BrickCracked: m_type = TileType::Empty; return true;
    case TileType::BrickStrong: m_type = TileType::BrickCracked; return true;
    default: return false;
    }
}
