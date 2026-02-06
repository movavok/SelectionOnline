#include "attackraycast.h"

#include <algorithm>

bool AttackRaycast::blocked(const QPointF& from, const QPointF& to, const Map& map) {
    QPointF dir = to - from;
    double length = QLineF(from, to).length();
    if (length <= 0.001) return false;

    const double step = 0.25;
    int steps = std::max(1, int(std::ceil(length / step)));

    for (int index = 0; index <= steps; ++index) {
        QPointF point = from + dir * (double(index) / steps);
        QPoint tilePos = map.worldToTile(point);
        if (!map.isInsideMap(tilePos)) return true;
        if (tileCollision(map.tileAt(tilePos.x(), tilePos.y()).getType()).projectileSolid)
            return true;
    }
    return false;
}

bool AttackRaycast::isProjectileBlocked(const QPointF& point, const Map& map) {
    const QPoint tile = map.worldToTile(point);
    if (!map.isInsideMap(tile)) return true;

    Tile::TileType tileType = map.tileAt(tile.x(), tile.y()).getType();
    return tileCollision(tileType).projectileSolid;
}

QPointF AttackRaycast::clampToObstacle(const QPointF& from, const QPointF& to, const Map& map) {
    const QPointF dir = to - from;
    const double distance = QLineF(from, to).length();

    if (distance <= 0.001) return to;

    double stepSize = 0.25;
    const int sampleCount = std::max(1, int(std::ceil(distance / stepSize)));

    QPointF lastFreePoint = from;
    if (isProjectileBlocked(from, map)) return lastFreePoint;

    for (int sampleIndex = 1; sampleIndex <= sampleCount; ++sampleIndex) {
        const double alpha = double(sampleIndex) / sampleCount;
        const QPointF currentPoint = from + dir * alpha;

        if (!isProjectileBlocked(currentPoint, map)) {
            lastFreePoint = currentPoint;
            continue;
        }

        QPointF low = lastFreePoint;
        QPointF high = currentPoint;

        for (int iteration = 0; iteration < 10; ++iteration) {
            const QPointF middle = (low + high) * 0.5;
            if (isProjectileBlocked(middle, map)) high = middle;
            else low = middle;
        }
        return high;
    }
    return to;
}
