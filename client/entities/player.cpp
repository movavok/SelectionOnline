#include "player.h"

Player::Player(const QPointF& startPos)
    : Entity(startPos, 15.0f)
{
    m_hp = m_maxHp = 400;
}

Player::~Player() {
    delete m_weapon;
}

bool Player::canAttack() const { return m_weapon && m_attackCooldown <= 0.f; }

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
    if (m_weapon) m_attackCooldown = m_weapon->getCooldown();
}

Player::AttackState Player::getAttackState() const { return m_attackState; }

void Player::setWeapon(Weapon* weapon) { delete m_weapon; m_weapon = weapon; }
const Weapon* Player::getWeapon() const { return m_weapon; }

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
