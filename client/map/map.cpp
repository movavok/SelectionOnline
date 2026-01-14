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
            case '#': type = Tile::TileType::Wall; break;
            default: type = Tile::TileType::Empty; break;
            }
            row.append(Tile(type));
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

void Map::tilesInRect(const QRectF& rect, QVector<QPoint>& out) const {
    const double mapWidth = static_cast<double>(m_tileCountX) * TILE_SIZE;
    const double mapHeight = static_cast<double>(m_tileCountY) * TILE_SIZE;
    const QRectF local = rect.translated(mapWidth / 2.0, mapHeight / 2.0);

    int startX = std::floor((local.left() + 1) / TILE_SIZE);
    int startY = std::floor((local.top() + 1) / TILE_SIZE);
    int endX = std::floor((local.right() - 1) / TILE_SIZE);
    int endY = std::floor((local.bottom() - 1) / TILE_SIZE);

    out.clear();
    for (int y = startY; y <= endY; ++y)
        for (int x = startX; x <= endX; ++x)
            out.append({x, y});
}

bool Map::intersectsAnyTiles(const QRectF& rect, const QVector<Tile::TileType>& types) const {
    QVector<QPoint> coords;
    tilesInRect(rect, coords);

    for (const QPoint& point : coords) {
        if (point.y() < 0 || point.y() >= m_tileCountY || point.x() < 0 || point.x() >= m_tileCountX)
            continue;
        if (types.contains(m_tilesGrid[point.y()][point.x()].getType()))
            return true;
    }
    return false;
}
