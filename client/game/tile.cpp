#include "tile.h"

Tile::Tile(TileType type) : m_type(type) {}

Tile::TileType Tile::getType() const { return m_type; }
