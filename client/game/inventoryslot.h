#ifndef INVENTORYSLOT_H
#define INVENTORYSLOT_H

#include "../map/tile.h"
#include "../combat/weapon.h"

struct InventorySlot {
    enum class SlotType { Empty, Resource, Weapon } m_type = SlotType::Empty;

    Tile::TileType m_resourceType;
    unsigned short m_resourceAmount = 0;

    Weapon* m_weapon = nullptr;

    static InventorySlot makeResource(Tile::TileType, unsigned short amount);
    static InventorySlot makeWeapon(Weapon*);
};

#endif // INVENTORYSLOT_H
