#ifndef MINIMAPWIDGET_H
#define MINIMAPWIDGET_H

#include <QWidget>
#include <QImage>
#include <QRectF>
#include <QPainter>

#include "../map/map.h"
#include "../game/fakesnapshot.h"

class MiniMapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MiniMapWidget(QWidget* parent = nullptr);

    void setMap(const Map*);
    void setSnapshot(const WorldSnapshot&);
    void setCameraRect(const QRectF&);

    void notifyTileChanged(int tileX, int tileY);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    const Map* m_map = nullptr;
    WorldSnapshot m_snapshot;
    QRectF m_cameraRect;

    QImage m_tiles; // 1px = 1 tile

    void rebuildTiles();
    void updateTilePixel(int tileX, int tileY);

    static QColor colorForTile(Tile::TileType);
    QPointF worldToMiniMap(const QPointF& worldPos, const QRectF& mmRect) const;
    void drawDot(const QPointF& worldPos, const QColor&, double radius, const QRectF& mmRect, QPainter&);
};

#endif // MINIMAPWIDGET_H
