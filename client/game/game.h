#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QList>
#include <QVector>
#include <algorithm>
#include <QRandomGenerator>

#include "../entities/player.h"
#include "../map/map.h"
#include "../map/pickupitem.h"
#include "../map/tilevisual.h"
#include "../combat/weaponmanager.h"
#include "attackraycast.h"
#include "hitinfo.h"
#include "attackfxutils.h"

class Game : public QObject
{
    Q_OBJECT
public:
    explicit Game(QObject* parent = nullptr);

    void update(float);

    void setPlayerInput(MoveDirection, bool);

    Player* getPlayer() const;
    const QList<Entity*>& getEntities() const;

    Map& getMap();
    const Map& getMap() const;
    QRectF getWorldBounds() const;

    const QList<PickupItem*>& getPickups() const;

    QPointF tileToWorld(const QPoint&) const;
    QPoint worldToTile(const QPointF&) const;

    bool canPlaceTile(const QPoint&) const;
    bool tryPlaceTile(const QPoint&);

    void maybeSpawnPickupForBrokenTile(const QPoint& tilePos, Tile::TileType oldType);
    bool removePickupAtTile(const QPoint& tilePos);

    QPainterPath getPlayerAttackShape(const QPointF&) const;

    void setExternalCollisionCircles(const QVector<QPointF>& centers, const QVector<float>& radii);
    void setExternalHitPlayers(const QVector<quint32>& playerIds, const QVector<QPointF>& centers, const QVector<float>& radii);

private:
    QPointF pickPlayerSpawn() const;
    void initPlayer(const QPointF& playerSpawn);

    Player* m_player = nullptr;
    QList<Entity*> m_entities;

    Map m_map;
    QRectF m_worldBounds;

    QList<PickupItem*> m_pickups;

    QVector<QPointF> m_externalCollisionCenters;
    QVector<float> m_externalCollisionRadii;

    QVector<quint32> m_externalHitPlayerIds;
    QVector<QPointF> m_externalHitCenters;
    QVector<float> m_externalHitRadii;

    bool canMove(const Entity*, const QPointF&) const;

    bool collidesWithSolidTiles(const QPointF& newPos, float radius) const;
    bool collidesWithEntities(const Entity* entity, const QPointF& newPos) const;
    bool collidesWithExternalCircles(const Entity* entity, const QPointF& newPos) const;

    QPainterPath cutSolidTiles(const QPainterPath&) const;
    void tryBreakTiles(const QPainterPath&);
    void tryBreakTileAt(const QPoint& tilePos, const QPainterPath& hitShape);
    void performWeaponHit(const Weapon&, const QPointF&, const QPainterPath&);
    void performWeaponHitOnEntities(const Weapon&, const QPainterPath&);
    void performWeaponHitOnExternalPlayers(const Weapon&, const QPainterPath&);
    void processPlayerAttack();

    void spawnPickupAtTile(const QPoint&, Tile::TileType);
    void applyPickup(PickupItem*);
    void checkPickupCollisions();

    QPainterPath makeCirclePath(const QPointF& center, float radius) const;

    void updateEntity(Entity* entity, float dt);
    void updatePlayer(Player* player, float dt);
    void updateNonPlayerEntity(Entity* entity, float dt);

signals:
    void tileChanged(int x, int y);
    void enemyHit(const HitInfo&);
    void externalPlayerHit(quint32 targetId, quint16 damage);
    void pickupCollected(qint16 tileX, qint16 tileY);
    void localPlayerAttackPerformed(float dirX, float dirY);
};

#endif // GAME_H
