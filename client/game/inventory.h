#ifndef INVENTORY_H
#define INVENTORY_H

#include "inventoryslot.h"

class Inventory
{
public:
    static constexpr int MAX_SLOTS = 5;

    void addResource(Tile::TileType type, int amount);
    bool spendResource(Tile::TileType type, int amount);

    void addWeapon(Weapon*);
    Weapon* getActiveWeapon() const;

    bool isActiveResource() const;
    Tile::TileType getActiveResourceType() const;

    void setActiveSlot(int index);
    int getActiveSlot() const;

private:
    QVector<InventorySlot> m_slots = QVector<InventorySlot>(MAX_SLOTS);
    unsigned short m_activeSlot = 0;
};

#endif // INVENTORY_H
