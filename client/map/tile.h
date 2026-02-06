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

    void setVariation(unsigned short);
    unsigned short getVariation() const;

    bool applyHit();

private:
    TileType m_type;
    unsigned char m_variation = 0;
};

#endif // TILE_H
