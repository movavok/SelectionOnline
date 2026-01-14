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

const Tile &Map::tileAt(int x, int y) const {
    static Tile emptyTile(Tile::TileType::Empty);
    if (y < 0 || y >= m_tileCountY || x < 0 || x >= m_tileCountX) return emptyTile;
    return m_tilesGrid[y][x];
}


