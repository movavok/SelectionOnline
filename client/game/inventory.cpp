#include "inventory.h"

void Inventory::addResource(Tile::TileType type, int amount) {
    if (amount <= 0) return;

    for (InventorySlot& slot : m_slots) {
        if (slot.m_type == InventorySlot::SlotType::Resource && slot.m_resourceType == type) {
            slot.m_resourceAmount += amount;
            return;
        }
    }

    m_slots.append(InventorySlot::makeResource(type, amount));
}

bool Inventory::spendResource(Tile::TileType type, int amount) {
    for (int i = 0; i < m_slots.size(); ++i) {
        InventorySlot& slot = m_slots[i];
        if (slot.m_type == InventorySlot::SlotType::Resource && slot.m_resourceType == type) {
            if (slot.m_resourceAmount < amount) return false;

            slot.m_resourceAmount -= amount;
            if (slot.m_resourceAmount == 0) m_slots.remove(i);
            return true;
        }
    }
    return false;
}

void Inventory::addWeapon(Weapon* weapon) {
    if (weapon) m_slots.append(InventorySlot::makeWeapon(weapon));
}

Weapon* Inventory::getActiveWeapon() const {
    if (m_activeSlot < 0 || m_activeSlot >= m_slots.size()) return nullptr;
    const InventorySlot& slot = m_slots[m_activeSlot];
    return (slot.m_type == InventorySlot::SlotType::Weapon) ? slot.m_weapon : nullptr;
}

void Inventory::setActiveSlot(int index) {
    if (index >= 0 && index < m_slots.size()) m_activeSlot = index;
}

int Inventory::getActiveSlot() const { return m_activeSlot; }

