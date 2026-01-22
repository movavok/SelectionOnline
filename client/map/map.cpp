#include "map.h"

unsigned short Map::getTileCountX() const { return m_tileCountX; }
unsigned short Map::getTileCountY() const { return m_tileCountY; }

bool Map::generateFromText(const QStringList& lines) {
    if (lines.isEmpty()) return false;

    m_tileCountY = lines.size();
    m_tileCountX = lines.first().size();

    m_tilesGrid.clear();
    m_tilesGrid.reserve(m_tileCountY);

    unsigned short rowIndex = 0;
    for (const QString& line : lines) {
        if (line.size() != m_tileCountX) {
            qWarning() << "Рядок #" << rowIndex << "невідповідної довжини";
            return false;
        }

        QVector<Tile> row;
        row.reserve(m_tileCountX);

        for (const QChar& symbol : line) {
            Tile::TileType type;
            switch (symbol.toLatin1()) {
            case 'd': type = Tile::TileType::Board; break;
            case 'b': type = Tile::TileType::BrickCracked; break;
            case 'B': type = Tile::TileType::BrickStrong; break;
            case 'g': type = Tile::TileType::Grass; break;
            case '~': type = Tile::TileType::Water; break;
            case '#': type = Tile::TileType::Wall; break;
            default: type = Tile::TileType::Empty; break;
            }
            Tile tile(type);
            if (type == Tile::TileType::Empty) {
                int random = rand() % 100;
                if (random < 85) tile.setVariation(0);
                else if (random < 95) tile.setVariation(1);
                else tile.setVariation(2);
            }

            row.append(tile);
        }
        m_tilesGrid.append(row);

        rowIndex++;
    }
    return true;
}

bool Map::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не вдалося відкрити файл: " << path;
        return false;
    }

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        line = line.trimmed();
        if (!line.isEmpty()) lines.append(line);
    }

    file.close();
    return generateFromText(lines);
}

const Tile& Map::tileAt(int x, int y) const {
    static Tile emptyTile(Tile::TileType::Empty);
    if (y < 0 || y >= m_tileCountY || x < 0 || x >= m_tileCountX) return emptyTile;
    return m_tilesGrid[y][x];
}

Tile& Map::tileAt(int x, int y) {
    return const_cast<Tile&>(static_cast<const Map&>(*this).tileAt(x, y));
}

QPoint Map::worldToTile(const QPointF& worldPos, const QRectF& worldBounds) {
    QPointF local = worldPos - worldBounds.topLeft();

    int x = int(local.x()) / TILE_SIZE;
    int y = int(local.y()) / TILE_SIZE;

    return QPoint(x, y);
}

QPointF Map::tileToWorld(const QPoint& tilePos, const QRectF& worldBounds) {
    return worldBounds.topLeft() +
           QPointF(tilePos.x() * TILE_SIZE, tilePos.y() * TILE_SIZE) +
           QPointF(TILE_SIZE / 2.0, TILE_SIZE / 2.0);
}

void Map::tilesInRect(const QRectF& rect, QVector<QPoint>& out) const {
    const double mapWidth = static_cast<double>(m_tileCountX) * TILE_SIZE;
    const double mapHeight = static_cast<double>(m_tileCountY) * TILE_SIZE;
    const QRectF local = rect.translated(mapWidth / 2.0, mapHeight / 2.0);

    int startX = std::floor(local.left() / TILE_SIZE);
    int startY = std::floor(local.top() / TILE_SIZE);
    int endX = std::floor(local.right() / TILE_SIZE);
    int endY = std::floor(local.bottom() / TILE_SIZE);

    out.clear();
    for (int y = startY; y <= endY; ++y)
        for (int x = startX; x <= endX; ++x)
            out.append({x, y});
}

bool Map::intersectsSolid(const QRectF& rect, CollisionActor actor) const {
    QVector<QPoint> tiles;
    tilesInRect(rect, tiles);

    for (QPoint& point : tiles) {
        const TileCollision& collision = tileCollision(tileAt(point.x(), point.y()).getType());

        if (actor == CollisionActor::Person && collision.personSolid) return true;
        if (actor == CollisionActor::Projectile && collision.projectileSolid) return true;
    }
    return false;
}

bool Map::intersectsGrass(const QRectF& rect) const {
    QVector<QPoint> tiles;
    tilesInRect(rect, tiles);

    for (const QPoint& point : tiles) {
        if (tileAt(point.x(), point.y()).getType() == Tile::TileType::Grass)
            return true;
    }
    return false;
}
