#ifndef ENTITY_H
#define ENTITY_H

#include <QPointF>
#include <QRectF>

class Entity
{
public:
    Entity(const QPointF& startPos);
    virtual ~Entity() = default;

    virtual void update(float) = 0;
    virtual QRectF bounds() const = 0;

    void setPosition(const QPointF&);
    QPointF getPosition() const;

    bool isAlive() const;
    void destroy();

protected:
    QPointF m_position;

    unsigned short m_width;
    unsigned short m_height;

    bool m_alive = true;
};

#endif // ENTITY_H
