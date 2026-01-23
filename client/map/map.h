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

    void setWorldBounds(int, int);
    QRectF getWorldBounds() const;

    bool loadFromFile(const QString&);

    const Tile& tileAt(int x, int y) const;
    Tile& tileAt(int x, int y);

    QPoint worldToTile(const QPointF&) const;
    QPointF tileToWorld(const QPoint&) const;

    void tilesInRect(const QRectF&, QVector<QPoint>& out) const;
    bool intersectsSolid(const QRectF&, CollisionActor) const;
    bool intersectsGrass(const QRectF&) const;
    bool isInsideMap(const QPoint&) const;

private:
    QRectF m_worldBounds;

    unsigned short m_tileCountX = 0;
    unsigned short m_tileCountY = 0;

    QVector<QVector<Tile>> m_tilesGrid;

    bool generateFromText(const QStringList&);
};

#endif // MAP_H
