#ifndef INVENTORY_H
#define INVENTORY_H

#include "inventoryslot.h"

class Inventory
{
public:
    void addResource(Tile::TileType type, int amount);
    bool spendResource(Tile::TileType type, int amount);

    void addWeapon(Weapon*);
    Weapon* getActiveWeapon() const;

    void setActiveSlot(int index);
    int getActiveSlot() const;

private:
    QVector<InventorySlot> m_slots;
    unsigned short m_activeSlot = 0;
};

#endif // INVENTORY_H
