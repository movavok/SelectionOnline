#ifndef MAP_H
#define MAP_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "tile.h"

class Map
{
public:
    static constexpr unsigned short TILE_SIZE = 32;

    unsigned short getTileCountX() const;
    unsigned short getTileCountY() const;

    bool loadFromFile(const QString&);
    const Tile& tileAt(int x, int y) const;

private:
    unsigned short m_tileCountX = 0;
    unsigned short m_tileCountY = 0;

    QVector<QVector<Tile>> m_tilesGrid;
    bool generateFromText(const QStringList&);
};

#endif // MAP_H
