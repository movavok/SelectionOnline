#ifndef PLAYERSTATEUPDATES_H
#define PLAYERSTATEUPDATES_H

#include <QString>

struct LocalPlayerStateUpdate {
    quint32 tick = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    quint16 hp = 0;
    quint16 maxHp = 0;
    quint8 activeItemKind = 0;
    quint8 activeResourceType = 0;
    float aimDirX = 1.0f;
    float aimDirY = 0.0f;
};

struct RemotePlayerStateUpdate {
    quint32 playerId = 0;
    quint32 tick = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    quint16 hp = 0;
    quint16 maxHp = 0;
    QString nickname;
    quint8 colorId = 255;
    quint8 activeItemKind = 0;
    quint8 activeResourceType = 0;
    float aimDirX = 1.0f;
    float aimDirY = 0.0f;
};

#endif // PLAYERSTATEUPDATES_H
