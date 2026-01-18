#ifndef GAME_H
#define GAME_H

#include <QList>

#include "../entities/player.h"
#include "../entities/enemy.h"
#include "../map/map.h"
#include "../map/pickupitem.h"
#include "../map/tilevisual.h"
#include "../combat/weaponmanager.h"

class Game
{
public:
    Game();

    void update(float);

    void setPlayerInput(MoveDirection, bool);

    Player* getPlayer() const;
    const QList<Entity*>& getEntities() const;

    const Map& getMap() const;
    QRectF getWorldBounds() const;

private:
    Player* m_player = nullptr;
    QList<Entity*> m_entities;

    Map m_map;
    QRectF m_worldBounds;

    QList<PickupItem> m_pickups;

    //helper
    bool canMove(const Entity*, const QPointF&) const;
    void spawnPickupAtTile(const QPointF&, Tile::TileType);
    void tryBreakTiles(const QPainterPath&);
    void performWeaponHit(const Weapon&, const QPointF&);
    void processPlayerAttack();
    void applyPickup(PickupItem&);
    void checkPickupCollisions();
};

#endif // GAME_H
