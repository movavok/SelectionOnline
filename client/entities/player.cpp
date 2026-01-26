#include "player.h"

Player::Player(const QPointF& startPos)
    : Entity(startPos, 15.0f)
{
    m_hp = m_maxHp = 400;
}

Player::~Player() {
    delete m_inventory.getActiveWeapon();
}

bool Player::canAttack() const { return m_inventory.getActiveWeapon() && m_attackCooldown <= 0.f; }

bool Player::consumeAttackRequest(QPointF& outDir) {
    if (m_attackRequested) {
        outDir = m_attackDir;
        m_attackRequested = false;
        return true;
    }
    return false;
}

void Player::startAiming() { m_attackState = AttackState::Aim; }

void Player::stopAiming(const QPointF& dir) {
    if (m_attackState == AttackState::Aim) {
        m_attackRequested = true;
        m_attackDir = dir;
        m_attackState = AttackState::Idle;
    }
}

void Player::onAttackPerformed() {
    if (m_inventory.getActiveWeapon()) m_attackCooldown = m_inventory.getActiveWeapon()->getCooldown();
}

Player::AttackState Player::getAttackState() const { return m_attackState; }

Inventory& Player::getInventory() { return m_inventory; }
const Inventory& Player::getInventory() const { return m_inventory; }

void Player::setActiveSlot(int index) { m_inventory.setActiveSlot(index); }
unsigned short Player::getActiveSlot() const { return m_inventory.getActiveSlotIndex(); }

const Weapon* Player::getActiveWeapon() const { return m_inventory.getActiveWeapon(); }

void Player::setInput(MoveDirection direction, bool pressed) {
    switch (direction) {
    case MoveDirection::MoveUp: m_movingUp = pressed; break;
    case MoveDirection::MoveDown: m_movingDown = pressed; break;
    case MoveDirection::MoveLeft: m_movingLeft = pressed; break;
    case MoveDirection::MoveRight: m_movingRight = pressed; break;
    }
}

QPointF Player::moveDistance(float dt) const {
    QPointF delta(0, 0);

    if (m_movingUp) delta.ry() -= m_speed * dt;
    if (m_movingDown) delta.ry() += m_speed * dt;
    if (m_movingLeft) delta.rx() -= m_speed * dt;
    if (m_movingRight) delta.rx() += m_speed * dt;

    return delta;
}

void Player::update(float deltaTime) {
    m_attackCooldown = std::max(0.f, m_attackCooldown - deltaTime);
}
