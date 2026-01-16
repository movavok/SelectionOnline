#ifndef PLAYER_H
#define PLAYER_H

#include "../input/inputtypes.h"
#include "entity.h"

class Player : public Entity
{
public:
    Player(const QPointF& startPos);

    void update(float) override;
    QRectF bounds() const override;

    unsigned short getCurrentHp() const;
    unsigned short getMaxHp() const;
    void setCurrentHp(unsigned short);

    void setInput(MoveDirection, bool);
    QPointF moveDistance(float) const;

private:
    float m_speed = 100.0f;

    unsigned short m_hp = 400;
    unsigned short m_maxHp = 400;

    bool m_movingUp = false;
    bool m_movingDown = false;
    bool m_movingLeft = false;
    bool m_movingRight = false;
};

#endif // PLAYER_H
