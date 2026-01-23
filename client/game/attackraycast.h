#ifndef ATTACKRAYCAST_H
#define ATTACKRAYCAST_H

#include <QLineF>

#include "../map/map.h"

class AttackRaycast {
public:
    static bool blocked(const QPointF& from, const QPointF& to, const Map&);
    static QPointF clampToObstacle(const QPointF& from, const QPointF& to, const Map&);
private:
    static bool isProjectileBlocked(const QPointF&, const Map&);
};

#endif // ATTACKRAYCAST_H
