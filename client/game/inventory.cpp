#include "inventory.h"

void Inventory::addResource(Tile::TileType type, int amount) {
    if (amount <= 0) return;

    for (InventorySlot& slot : m_slots) {
        if (slot.m_type == InventorySlot::SlotType::Resource && slot.m_resourceType == type) {
            slot.m_resourceAmount += amount;
            return;
        }
    }

    for (int index = 1; index < MAX_SLOTS; ++index) {
        if (m_slots[index].m_type == InventorySlot::SlotType::Empty) {
            m_slots[index] = InventorySlot::makeResource(type, amount);
            return;
        }
    }
}

bool Inventory::spendResource(Tile::TileType type, int amount) {
    for (int i = 0; i < MAX_SLOTS; ++i) {
        InventorySlot& slot = m_slots[i];
        if (slot.m_type == InventorySlot::SlotType::Resource && slot.m_resourceType == type) {
            if (slot.m_resourceAmount < amount) return false;

            slot.m_resourceAmount -= amount;
            if (slot.m_resourceAmount == 0) slot = InventorySlot();
            return true;
        }
    }
    return false;
}

void Inventory::addWeapon(Weapon* weapon) {
    if (weapon) {
        for (int index = 0; index < MAX_SLOTS; ++index) {
            if (m_slots[index].m_type == InventorySlot::SlotType::Empty) {
                m_slots[index] = InventorySlot::makeWeapon(weapon);
                return;
            }
        }
    }
    delete weapon;
}

Weapon* Inventory::getActiveWeapon() const {
    if (m_activeSlot < 0 || m_activeSlot >= MAX_SLOTS) return nullptr;
    const InventorySlot& slot = m_slots[m_activeSlot];
    return (slot.m_type == InventorySlot::SlotType::Weapon) ? slot.m_weapon : nullptr;
}

bool Inventory::isActiveResource() const {
    const InventorySlot& slot = m_slots[m_activeSlot];
    return slot.m_type == InventorySlot::SlotType::Resource;
}

Tile::TileType Inventory::getActiveResourceType() const {
    const InventorySlot& slot = m_slots[m_activeSlot];
    return slot.m_resourceType;
}

void Inventory::setActiveSlot(int index) {
    if (index >= 0 && index < MAX_SLOTS) m_activeSlot = index;
}

int Inventory::getActiveSlotIndex() const { return m_activeSlot; }

const InventorySlot& Inventory::getSlot(int index) const {
    static InventorySlot empty;
    if (index < 0 || index >= MAX_SLOTS) return empty;
    return m_slots[index];
}

