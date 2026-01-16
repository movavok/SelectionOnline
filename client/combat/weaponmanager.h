#ifndef WEAPONMANAGER_H
#define WEAPONMANAGER_H

#include "katana.h"

class WeaponManager
{
public:
    static Weapon* create(const QString& name);
};

#endif // WEAPONMANAGER_H
