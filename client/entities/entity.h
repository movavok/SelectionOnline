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
    void setPrevPosition(const QPointF&);
    QPointF getPrevPosition() const;

    void setImpulse(const QPointF&);
    void addImpulse(const QPointF&);
    const QPointF& getImpulse() const;

    unsigned short getCurrentHp() const;
    unsigned short getMaxHp() const;

    void takeDamage(int);

    bool isAlive() const;
    void destroy();

protected:
    QPointF m_position;
    QPointF m_prevPosition;

    QPointF m_impulse = QPointF(0, 0);

    float m_radius;

    unsigned short m_hp;
    unsigned short m_maxHp;
};

#endif // ENTITY_H
