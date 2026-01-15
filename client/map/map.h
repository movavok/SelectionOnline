#ifndef MAP_H
#define MAP_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QPointF>
#include <QRectF>
#include <QDebug>

#include "tilecollision.h"

class Map
{
public:
    enum class CollisionActor { Person, Projectile };

    static constexpr unsigned short TILE_SIZE = 32;

    unsigned short getTileCountX() const;
    unsigned short getTileCountY() const;

    bool loadFromFile(const QString&);
    const Tile& tileAt(int x, int y) const;

    bool intersectsSolid(const QRectF&, CollisionActor) const;

private:
    unsigned short m_tileCountX = 0;
    unsigned short m_tileCountY = 0;

    QVector<QVector<Tile>> m_tilesGrid;

    bool generateFromText(const QStringList&);
    void tilesInRect(const QRectF&, QVector<QPoint>& out) const;
};

#endif // MAP_H
