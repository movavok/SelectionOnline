#ifndef FAKESERVER_H
#define FAKESERVER_H

#include <QHash>
#include <QPointF>
#include <QVector>

#include "fakesnapshot.h"
#include "game.h"

class Entity;
class Player;

class FakeServer {
public:
    FakeServer();

    void update(float);

    void setPlayerInput(MoveDirection, bool);
    void setActiveSlot(int);

    void startAiming();
    void stopAiming(const QPointF& attackDirWorld);

    bool tryPlaceTile(const QPoint&);

    WorldSnapshot makeSnapshot(const QPointF& mouseScene);

    void setExternalCollisionCircles(const QVector<QPointF>& centers, const QVector<float>& radii);
    void setExternalHitPlayers(const QVector<quint32>& playerIds, const QVector<QPointF>& centers, const QVector<float>& radii);

    Game& game();
    const Game& game() const;

    quint32 entityIdFor(const Entity*) const;

private:
    EntitySnapshot makeEntitySnapshot(Entity* entity, const Player* localPlayer, const QPointF& mouseScene);
    void fillEntitySnapshotCommon(EntitySnapshot& entitySnapshot, const Entity* entity) const;
    void fillPlayerSnapshot(EntitySnapshot& entitySnapshot, const Player& player) const;
    void fillLocalPlayerSnapshot(EntitySnapshot& entitySnapshot, const Player& localPlayer, const QPointF& mouseScene);

    void ensureEntityIds();
    quint32 ensureIdFor(const Entity*);

    Game m_game;
    QHash<const Entity*, quint32> m_entityIds;
    quint32 m_nextId = 1;
};

#endif // FAKESERVER_H
