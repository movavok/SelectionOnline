#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QList>

#include "../entities/player.h"
#include "../entities/enemy.h"
#include "../map/map.h"
#include "../map/pickupitem.h"
#include "../map/tilevisual.h"
#include "../combat/weaponmanager.h"

class Game : public QObject
{
    Q_OBJECT
public:
    explicit Game(QObject* parent = nullptr);

    void update(float);

    void setPlayerInput(MoveDirection, bool);

    Player* getPlayer() const;
    const QList<Entity*>& getEntities() const;

    const Map& getMap() const;
    QRectF getWorldBounds() const;

    const QList<PickupItem*>& getPickups() const;

    QPointF tileToWorld(const QPoint&) const;
    QPoint worldToTile(const QPointF&) const;

    bool canPlaceTile(const QPoint&, Tile::TileType) const;
    bool tryPlaceTile(const QPoint&);

private:
    Player* m_player = nullptr;
    QList<Entity*> m_entities;

    Map m_map;
    QRectF m_worldBounds;

    QList<PickupItem*> m_pickups;

    //helper
    bool canMove(const Entity*, const QPointF&) const;

    void tryBreakTiles(const QPainterPath&);
    void performWeaponHit(const Weapon&, const QPointF&);
    void processPlayerAttack();

    void spawnPickupAtTile(const QPoint&, Tile::TileType);
    void applyPickup(PickupItem*);
    void checkPickupCollisions();

signals:
    void tileChanged(int x, int y);
};

#endif // GAME_H
