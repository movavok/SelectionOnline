#ifndef WEAPON_H
#define WEAPON_H

#include <QPainterPath>

#include "../entities/entity.h"

class Weapon {
public:
    virtual ~Weapon() = default;
    virtual QPainterPath indicatorShape(const Entity&) const = 0;
    virtual double insideOffset(const Entity&) const = 0;
};

#endif // WEAPON_H
