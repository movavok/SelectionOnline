#ifndef ENTITY_H
#define ENTITY_H

#include <QPointF>
#include <QPainterPath>

class Entity
{
public:
    Entity(const QPointF& startPos, float radius);
    virtual ~Entity() = default;

    virtual void update(float) = 0;

    virtual float getRadius() const;

    void setPosition(const QPointF&);
    QPointF getPosition() const;

    unsigned short getCurrentHp() const;
    unsigned short getMaxHp() const;

    bool isAlive() const;
    void destroy();

protected:
    QPointF m_position;
    float m_radius;

    unsigned short m_hp;
    unsigned short m_maxHp;

    bool m_alive = true;
};

#endif // ENTITY_H
