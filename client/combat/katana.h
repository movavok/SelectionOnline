#ifndef KATANA_H
#define KATANA_H

#include "weapon.h"

class Katana : public Weapon {
public:
    Katana();

    QPixmap getIcon() const override;
    QSize getIconSize() const override;

    QPointF getGripPoint() const override;

    QPainterPath indicatorShape(const Entity&) const override;
    double getWeaponRange() const override;
    double insideOffset(const Entity&) const override;
};

#endif // KATANA_H
