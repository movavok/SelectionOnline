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

    void setInput(MoveDirection, bool);
    QPointF moveDistance(float) const;

private:
    float m_speed = 100.0f;
    int m_hp = 100;

    bool m_movingUp = false;
    bool m_movingDown = false;
    bool m_movingLeft = false;
    bool m_movingRight = false;
};

#endif // PLAYER_H
