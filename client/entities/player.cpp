#include "player.h"

Player::Player(const QPointF& startPos)
    : Entity(startPos)
{
    m_width = 30;
    m_height = 30;
}

void Player::setInput(MoveDirection direction, bool pressed)
{
    switch (direction) {
    case MoveDirection::MoveUp: m_movingUp = pressed; break;
    case MoveDirection::MoveDown: m_movingDown = pressed; break;
    case MoveDirection::MoveLeft: m_movingLeft = pressed; break;
    case MoveDirection::MoveRight: m_movingRight = pressed; break;
    }
}

void Player::update(float deltaTime)
{
    QPointF delta(0.0f, 0.0f);

    if (m_movingUp) delta.ry() -= m_speed * deltaTime;
    if (m_movingDown) delta.ry() += m_speed * deltaTime;
    if (m_movingLeft) delta.rx() -= m_speed * deltaTime;
    if (m_movingRight) delta.rx() += m_speed * deltaTime;

    m_position += delta;
}

QRectF Player::bounds() const { return QRectF(m_position.x() - (m_width / 2), m_position.y() - (m_height / 2), m_width, m_height); }
