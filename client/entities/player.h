#ifndef PLAYER_H
#define PLAYER_H

#include "../combat/weapon.h"
#include "../input/inputtypes.h"
#include "entity.h"

class Player : public Entity
{
public:
    Player(const QPointF& startPos);
    ~Player();

    void update(float) override;
    QRectF bounds() const override;

    unsigned short getCurrentHp() const;
    unsigned short getMaxHp() const;
    void setCurrentHp(unsigned short);

    void setInput(MoveDirection, bool);
    QPointF moveDistance(float) const;

    enum class AttackState { Idle, Aim };
    void setWeapon(Weapon*);
    const Weapon* getWeapon() const;

    void startAiming();
    void stopAiming();

    AttackState getAttackState() const;

private:
    float m_speed = 100.0f;

    unsigned short m_hp = 400;
    unsigned short m_maxHp = 400;

    AttackState m_attackState = AttackState::Idle;
    Weapon* m_weapon = nullptr;

    bool m_movingUp = false;
    bool m_movingDown = false;
    bool m_movingLeft = false;
    bool m_movingRight = false;
};

#endif // PLAYER_H
