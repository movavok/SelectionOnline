#include "weapon.h"

Weapon::Weapon(unsigned short damage, float cooldown)
    : m_damage(damage), m_cooldown(cooldown) {}

unsigned short Weapon::getDamage() const { return m_damage; }
float Weapon::getCooldown() const { return m_cooldown; }
