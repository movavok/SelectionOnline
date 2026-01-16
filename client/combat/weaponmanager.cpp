#include "weaponmanager.h"

Weapon* WeaponManager::create(const QString& name) {
    if (name == "Katana") return new Katana();
    return nullptr;
}
