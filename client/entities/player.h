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

    void setInput(MoveDirection, bool);
    QPointF moveDistance(float) const;

    enum class AttackState { Idle, Aim };
    void setWeapon(Weapon*);
    const Weapon* getWeapon() const;

    AttackState getAttackState() const;

    void startAiming();
    void stopAiming(const QPointF&);

    bool consumeAttackRequest(QPointF&);
    bool canAttack() const;

    void onAttackPerformed();

private:
    float m_speed = 100.0f;

    Weapon* m_weapon = nullptr;

    AttackState m_attackState = AttackState::Idle;
    bool m_attackRequested = false;
    QPointF m_attackDir;
    float m_attackCooldown = 0.0f;

    bool m_movingUp = false;
    bool m_movingDown = false;
    bool m_movingLeft = false;
    bool m_movingRight = false;
};

#endif // PLAYER_H
