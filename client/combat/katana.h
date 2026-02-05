#ifndef KATANA_H
#define KATANA_H

#include "weapon.h"

class Katana : public Weapon {
public:
    Katana();

    QPixmap getIcon() const override;

    QPixmap getSprite() const override;
    QSize getSpriteSize() const override;
    QPointF getGripPoint() const override;

    QPainterPath indicatorShape(const Entity&) const override;
    QPainterPath indicatorShapeForRadius(float entityRadius) const;

    double getWeaponRange() const override;

    double insideOffset(const Entity&) const override;
    double insideOffsetForRadius(float entityRadius) const;
};

#endif // KATANA_H
