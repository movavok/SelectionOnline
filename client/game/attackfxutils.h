#ifndef ATTACKFXUTILS_H
#define ATTACKFXUTILS_H

#include <QPainterPath>
#include <QPointF>
#include <QSizeF>
#include <cmath>
#include <limits>

#include "attackraycast.h"
#include "../combat/katana.h"
#include "../entities/entity.h"
#include "../map/map.h"

class Map;

class AttackFxUtils final {
public:
    struct Params {
        bool valid = false;
        QPointF centerOffset;
        QSizeF size;
        double angleDeg = 0.0;
    };

    static QPainterPath clipAttackPathToObstacles(const QPainterPath&, const QPointF&, const Map&);

    static Params computeAttackFxParamsFromShape(const QPointF&, const QPointF& dirVector, const QPainterPath& shapeWorld);

    static QPainterPath buildKatanaIndicatorAttackPathWorld(const QPointF&, const QPointF& dirVector, float entityRadius);
};

#endif // ATTACKFXUTILS_H
