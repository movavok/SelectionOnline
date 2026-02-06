#ifndef HITINFO_H
#define HITINFO_H

#include <QPointF>

#include "../entities/entity.h"

struct HitInfo {
    Entity* attacker = nullptr;
    Entity* target = nullptr;
    int damage = 0;
    QPointF direction;
};

#endif // HITINFO_H
