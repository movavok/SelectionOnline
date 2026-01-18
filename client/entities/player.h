#ifndef PLAYER_H
#define PLAYER_H

#include "../combat/weapon.h"
#include "../input/inputtypes.h"
#include "entity.h"
#include "../game/inventory.h"

class Player : public Entity
{
public:
    Player(const QPointF& startPos);
    ~Player();

    void update(float) override;

    void setInput(MoveDirection, bool);
    QPointF moveDistance(float) const;

    Inventory& getInventory();
    const Inventory& getInventory() const;

    const Weapon* getActiveWeapon() const;

    void setActiveSlot(int);
    unsigned short getActiveSlot() const;

    enum class AttackState { Idle, Aim };
    AttackState getAttackState() const;

    void startAiming();
    void stopAiming(const QPointF&);

    bool consumeAttackRequest(QPointF&);
    bool canAttack() const;

    void onAttackPerformed();

private:
    float m_speed = 100.0f;

    Inventory m_inventory;

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
