#ifndef WEAPON_H
#define WEAPON_H

#include <QPainterPath>

#include "../entities/entity.h"

class Weapon {
public:
    Weapon(unsigned short damage, float cooldown);
    virtual ~Weapon() = default;

    virtual QPainterPath indicatorShape(const Entity&) const = 0;
    virtual double insideOffset(const Entity&) const = 0;

    unsigned short getDamage() const;
    float getCooldown() const;

protected:
    unsigned short m_damage;
    float m_cooldown;
};

#endif // WEAPON_H
