#include "katana.h"

Katana::Katana() : Weapon(75, 0.5f)
{
    m_iconImage = QPixmap(":/ui/icons/katana.png");

    m_weaponSprite = QPixmap(":/weapons/katana.png");
    m_spriteWidth = 6;
    m_spriteHeight = 36;

    m_weaponRange = 60;
}

QPixmap Katana::getIcon() const { return m_iconImage; }

QPixmap Katana::getSprite() const { return m_weaponSprite;  }

QSize Katana::getSpriteSize() const { return QSize(m_spriteWidth, m_spriteHeight); }

QPainterPath Katana::indicatorShape(const Entity& person) const {
    const double insideRadius = insideOffset(person);
    const double outsideRadius = m_weaponRange;

    const double angleDeg = 90.0;
    const double startAngle = -angleDeg / 2.0;

    QPainterPath path;

    QRectF outsideRect(-outsideRadius, -outsideRadius,
                     outsideRadius * 2, outsideRadius * 2);
    path.arcMoveTo(outsideRect, startAngle);
    path.arcTo(outsideRect, startAngle, angleDeg);

    QRectF insideRect(-insideRadius, -insideRadius,
                      insideRadius * 2, insideRadius * 2);
    path.arcTo(insideRect, startAngle + angleDeg, -angleDeg);

    path.closeSubpath();
    return path;
}

double Katana::insideOffset(const Entity& person) const { return person.getRadius() + 6; }

double Katana::getWeaponRange() const { return m_weaponRange; }

QPointF Katana::getGripPoint() const { return QPointF(2, m_spriteHeight / 2); }
