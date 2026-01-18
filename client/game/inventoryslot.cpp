#include "inventoryslot.h"

InventorySlot InventorySlot::makeResource(Tile::TileType type, unsigned short amount) {
    InventorySlot slot;
    slot.m_type = SlotType::Resource;
    slot.m_resourceType = type;
    slot.m_resourceAmount = amount;
    slot.m_weapon = nullptr;
    return slot;
}

InventorySlot InventorySlot::makeWeapon(Weapon* weapon) {
    InventorySlot slot;
    slot.m_type = SlotType::Weapon;
    slot.m_weapon = weapon;
    slot.m_resourceAmount = 0;
    return slot;
}
