#ifndef KATANA_H
#define KATANA_H

#include "weapon.h"

class Katana : public Weapon {
public:
    QPainterPath indicatorShape(const Entity&) const override;
    double insideOffset(const Entity&) const override;
};

#endif // KATANA_H
