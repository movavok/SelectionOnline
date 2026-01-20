#ifndef TILE_H
#define TILE_H

class Tile
{
public:
    enum class TileType {
        Empty,
        Board,
        BrickCracked,
        BrickStrong,
        Grass,
        Water,
        Wall
    };

    Tile(TileType type = TileType::Empty);

    void setType(TileType);
    TileType getType() const;

    bool applyHit();

private:
    TileType m_type;
};

#endif // TILE_H
