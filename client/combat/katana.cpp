#include "katana.h"

QPainterPath Katana::indicatorShape(const Entity& person) const {
    const double insideRadius = insideOffset(person);
    const double outsideRadius = 60.0;

    const double angleDeg = 90.0;
    const double startAngle = -angleDeg / 2.0;

    QPainterPath path;

    QRectF outsideRect(-outsideRadius, -outsideRadius,
                     outsideRadius * 2, outsideRadius * 2);
    path.arcMoveTo(outsideRect, startAngle);
    path.arcTo(outsideRect, startAngle, angleDeg);

    QRectF insideRect(-insideRadius, -insideRadius,
                      insideRadius * 2, insideRadius * 2);
    path.arcTo(insideRect, startAngle + angleDeg, -angleDeg);

    path.closeSubpath();
    return path;
}

double Katana::insideOffset(const Entity& person) const { return person.getWidth() / 2 + 6; }
