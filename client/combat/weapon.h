#ifndef WEAPON_H
#define WEAPON_H

#include <QPainterPath>
#include <QPixmap>

#include "../entities/entity.h"

class Weapon {
public:
    Weapon(unsigned short damage, float cooldown);
    virtual ~Weapon() = default;

    virtual QPixmap getIcon() const = 0;
    virtual QSize getIconSize() const = 0;
    virtual QPointF getGripPoint() const = 0;

    virtual QPainterPath indicatorShape(const Entity&) const = 0;
    virtual double insideOffset(const Entity&) const = 0;

    unsigned short getDamage() const;
    float getCooldown() const;

protected:
    unsigned short m_damage;
    float m_cooldown;

    QPixmap m_iconSprite;

    unsigned short m_iconWidth;
    unsigned short m_iconHeight;
};

#endif // WEAPON_H
