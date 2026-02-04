#include "attackfxutils.h"

QPainterPath AttackFxUtils::clipAttackPathToObstacles(const QPainterPath& attackPath, const QPointF& playerPos, const Map& gameMap) {
    QPainterPath resultPath;

    for (const QPolygonF& polygon : attackPath.toSubpathPolygons()) {
        if (polygon.size() < 3) continue;

        QPolygonF clippedPolygon;
        clippedPolygon.reserve(polygon.size());
        for (const QPointF& point : polygon)
            clippedPolygon << AttackRaycast::clampToObstacle(playerPos, point, gameMap);

        if (clippedPolygon.size() < 3) continue;

        const QRectF bounds = clippedPolygon.boundingRect();
        if (bounds.width() < 0.5 && bounds.height() < 0.5)
            continue;

        resultPath.addPolygon(clippedPolygon);
    }

    return resultPath;
}

AttackFxUtils::Params AttackFxUtils::computeAttackFxParamsFromShape(const QPointF& playerPos, const QPointF& dirVector, const QPainterPath& shapeWorld) {
    Params resultParams;

    const double dirLength = std::hypot(dirVector.x(), dirVector.y());
    if (dirLength <= 0.0001) return resultParams;

    const double angleRadians = std::atan2(dirVector.y(), dirVector.x());
    const double angleDegrees = angleRadians * 180.0 / M_PI;

    const double cosAngle = std::cos(angleRadians);
    const double sinAngle = std::sin(angleRadians);

    double minX = std::numeric_limits<double>::infinity();
    double minY = std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();
    double maxY = -std::numeric_limits<double>::infinity();

    const QList<QPolygonF> polygons = shapeWorld.toSubpathPolygons();
    for (const QPolygonF& polygon : polygons)
        for (const QPointF& pointWorld : polygon) {
            const QPointF localPoint = pointWorld - playerPos;

            const double alignedX =  localPoint.x() * cosAngle + localPoint.y() * sinAngle;
            const double alignedY = -localPoint.x() * sinAngle + localPoint.y() * cosAngle;

            minX = std::min(minX, alignedX);
            minY = std::min(minY, alignedY);
            maxX = std::max(maxX, alignedX);
            maxY = std::max(maxY, alignedY);
        }

    if (!std::isfinite(minX) || !std::isfinite(minY) || !std::isfinite(maxX) || !std::isfinite(maxY))
        return resultParams;

    const double alignedWidth = std::max(0.0, maxX - minX);
    const double alignedHeight = std::max(0.0, maxY - minY);
    const QPointF centerAligned(minX + alignedWidth * 0.5, minY + alignedHeight * 0.5);

    const QPointF centerOffset(centerAligned.x() * cosAngle - centerAligned.y() * sinAngle,
                               centerAligned.x() * sinAngle + centerAligned.y() * cosAngle);

    resultParams.valid = true;
    resultParams.angleDeg = angleDegrees;
    resultParams.size = QSizeF(alignedWidth, alignedHeight);
    resultParams.centerOffset = centerOffset;
    return resultParams;
}

QPainterPath AttackFxUtils::buildKatanaIndicatorAttackPathWorld(const QPointF& playerPos, const QPointF& dirVector, float entityRadius) {
    const double dirLength = std::hypot(dirVector.x(), dirVector.y());
    if (dirLength <= 0.0001) return {};

    const double angleDegrees = std::atan2(dirVector.y(), dirVector.x()) * 180.0 / M_PI;

    Katana katana;

    QTransform rotation;
    rotation.rotate(angleDegrees);

    QPainterPath attackPath = rotation.map(katana.indicatorShapeForRadius(entityRadius));
    attackPath.translate(playerPos);
    return attackPath;
}
