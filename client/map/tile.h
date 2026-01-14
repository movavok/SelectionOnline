#ifndef TILE_H
#define TILE_H

class Tile
{
public:
    enum class TileType {
        Empty,
        Wall
    };

    Tile(TileType type = TileType::Empty);

    TileType getType() const;

private:
    TileType m_type;
};

#endif // TILE_H
