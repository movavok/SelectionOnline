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

    void takeDamage(int);

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

    AttackState m_attackState = AttackState::Idle;
    Weapon* m_weapon = nullptr;

    bool m_movingUp = false;
    bool m_movingDown = false;
    bool m_movingLeft = false;
    bool m_movingRight = false;
};

#endif // PLAYER_H
