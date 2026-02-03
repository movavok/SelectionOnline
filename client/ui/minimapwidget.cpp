#include "minimapwidget.h"

MiniMapWidget::MiniMapWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(180, 180);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

void MiniMapWidget::setMap(const Map* map) {
    m_map = map;
    rebuildTiles();
    update();
}

void MiniMapWidget::setSnapshot(const WorldSnapshot& snapshot) {
    m_snapshot = snapshot;
    update();
}

void MiniMapWidget::setCameraRect(const QRectF& rect) {
    m_cameraRect = rect;
    update();
}

void MiniMapWidget::notifyTileChanged(int tileX, int tileY) {
    updateTilePixel(tileX, tileY);
    update();
}

QColor MiniMapWidget::colorForTile(Tile::TileType type) {
    switch (type) {
    case Tile::TileType::Wall: return QColor(90, 90, 90);
    case Tile::TileType::Water: return QColor(60, 120, 220);
    case Tile::TileType::Grass: return QColor(60, 180, 80);
    case Tile::TileType::Board: return QColor(170, 140, 90);
    case Tile::TileType::BrickStrong: return QColor(170, 70, 60);
    case Tile::TileType::BrickCracked: return QColor(200, 120, 100);
    default: return QColor(100, 200, 100);
    }
}

void MiniMapWidget::rebuildTiles() {
    if (!m_map) {
        m_tiles = QImage();
        return;
    }

    const int width = int(m_map->getTileCountX());
    const int height = int(m_map->getTileCountY());
    if (width <= 0 || height <= 0) {
        m_tiles = QImage();
        return;
    }

    m_tiles = QImage(width, height, QImage::Format_RGB32);

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            m_tiles.setPixelColor(x, y, colorForTile(m_map->tileAt(x, y).getType()));
}

void MiniMapWidget::updateTilePixel(int tileX, int tileY) {
    if (!m_map) return;
    if (m_tiles.isNull()) return;

    if (!m_map->isInsideMap(QPoint(tileX, tileY))) return;

    m_tiles.setPixelColor(tileX, tileY, colorForTile(m_map->tileAt(tileX, tileY).getType()));
}

QPointF MiniMapWidget::worldToMiniMap(const QPointF& worldPos, const QRectF& mmRect) const {
    if (!m_map) return mmRect.center();

    const QRectF worldBounds = m_map->getWorldBounds();
    const double denomX = worldBounds.width();
    const double denomY = worldBounds.height();

    const double normX = (worldPos.x() - worldBounds.left()) / denomX;
    const double normY = (worldPos.y() - worldBounds.top()) / denomY;

    return QPointF(mmRect.left() + normX * mmRect.width(),
                   mmRect.top() + normY * mmRect.height());
}

void MiniMapWidget::drawDot(const QPointF& world, const QColor& color, double radius, const QRectF& mmRect, QPainter& painter) {
    QPointF entityPos = worldToMiniMap(world, mmRect);
    painter.setPen(Qt::NoPen);
    QColor entityColor = color;
    entityColor.setAlpha(220);
    painter.setBrush(entityColor);
    painter.drawEllipse(entityPos, radius, radius);
}

void MiniMapWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF mmRect = QRectF(rect());

    if (m_map && m_tiles.isNull())
        rebuildTiles();

    // map
    if (!m_tiles.isNull()) {
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setOpacity(0.55);
        painter.drawImage(mmRect, m_tiles);
        painter.setOpacity(1.0);
        painter.setRenderHint(QPainter::Antialiasing, true);
    }

    // camera rect
    if (m_map && !m_cameraRect.isNull()) {
        QRectF cam(worldToMiniMap(m_cameraRect.topLeft(), mmRect), worldToMiniMap(m_cameraRect.bottomRight(), mmRect));
        cam = cam.normalized();
        painter.setPen(QPen(QColor(255, 255, 255, 150), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(cam);
    }

    // entities
    for (const EntitySnapshot& entity : m_snapshot.entities) {
        if (!entity.alive) continue;

        if (m_map && !entity.isLocalPlayer) {
            const float radius = entity.radius;
            const QRectF entityRect(entity.pos - QPointF(radius, radius), QSizeF(radius * 2.0, radius * 2.0));
            if (m_map->intersectsGrass(entityRect))
                continue;
        }

        QColor color = entity.uiColor;
        float radius = 0;
        if (entity.isLocalPlayer) radius = 3.3f;
        else radius = 2.4f;
        drawDot(entity.pos, color, radius, mmRect, painter);
    }
}
