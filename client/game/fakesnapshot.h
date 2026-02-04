#ifndef FAKESNAPSHOT_H
#define FAKESNAPSHOT_H

#include <QColor>
#include <QPainterPath>
#include <QPointF>
#include <QPixmap>
#include <QString>
#include <QVector>

enum class SnapshotEntityKind : quint8 {
    Player = 1,
    Enemy = 2
};

struct EntitySnapshot {
    quint32 id = 0;
    SnapshotEntityKind kind = SnapshotEntityKind::Enemy;

    bool alive = false;
    QPointF pos;
    QPointF prevPos;
    QPointF aimDir;
    float radius = 0.0f;

    quint16 hp = 0;
    quint16 maxHp = 0;

    QString nickname;
    QColor uiColor;

    bool isLocalPlayer = false;

    bool showAttackIndicator = false;
    bool canAttack = false;
    QPainterPath attackIndicatorLocalPath;

    bool showSlotIndicator = false;
    QPixmap slotIndicatorPixmap;
};

struct WorldSnapshot {
    QVector<EntitySnapshot> entities;
};

#endif // FAKESNAPSHOT_H
