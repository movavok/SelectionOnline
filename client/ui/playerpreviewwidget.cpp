#include "playerpreviewwidget.h"

PlayerPreviewWidget::PlayerPreviewWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_background = QPixmap(":/ui/player_preview_bg.png");

    update();
}

void PlayerPreviewWidget::setColor(const QColor &color) {
    if (m_color == color) return;
    m_color = color;
    update();
}

void PlayerPreviewWidget::setWeapon(WeaponType type) {
    if (m_weaponType == type) return;
    m_weaponType = type;

    switch (m_weaponType) {
    case WeaponType::Katana: m_weaponSprite = QPixmap(":/weapons/katana.png"); break;
    case WeaponType::Empty:
    default: m_weaponSprite = QPixmap(); break;
    }

    update();
}

void PlayerPreviewWidget::setAbility(AbilityType type) {
    if (m_abilityType == type) return;
    m_abilityType = type;

    switch (m_abilityType) {
    case AbilityType::MirrorShield: m_skinSprite = QPixmap(":/entities/tank.png"); break;
    case AbilityType::Empty:
    default: m_skinSprite = QPixmap(); break;
    }

    update();
}

void PlayerPreviewWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int squareSize = qMin(width(), height());
    const QRect squareRect((width() - squareSize) / 2, (height() - squareSize) / 2, squareSize, squareSize);

    if (!m_background.isNull())
        painter.drawPixmap(squareRect, m_background.scaled(squareRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    QPointF center = squareRect.center();
    const QSizeF circleSize = QSizeF(squareSize, squareSize) * 0.28;
    const QSizeF weaponSize = QSizeF(squareSize * 0.55, squareSize * 0.55);
    const QSizeF playerSize = QSizeF(squareSize, squareSize) * 0.55;
    const QPointF weaponCenter = center + QPointF(0, squareSize * 0.25);

    if (m_color != Qt::transparent) drawColoredCircle(painter, center, circleSize);
    if (m_abilityType != AbilityType::Empty) drawPlayerSkin(painter, center, playerSize);
    if (m_weaponType != WeaponType::Empty) drawWeapon(painter, weaponCenter, weaponSize);
}

QSize PlayerPreviewWidget::sizeHint() const { return QSize(160, 160); }
QSize PlayerPreviewWidget::minimumSizeHint() const { return QSize(48, 48); }
bool PlayerPreviewWidget::hasHeightForWidth() const { return true; }
int PlayerPreviewWidget::heightForWidth(int width) const { return width; }


void PlayerPreviewWidget::drawColoredCircle(QPainter& painter, const QPointF& center, const QSizeF& size) {
    QColor color = m_color;
    color.setAlpha(150);

    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, size.width(), size.height());
}

QRectF centeredRect(const QPointF& center, const QSizeF& size) {
    return QRectF(center.x() - size.width()  / 2.0, center.y() - size.height() / 2.0, size.width(), size.height());
}

void drawScaledPixmap(QPainter& painter, const QPixmap& pix, const QPointF& center, const QSizeF& size) {
    if (pix.isNull()) return;

    const int targetWidth = qMax(1, (int)size.width());
    const int targetHeight = qMax(1, (int)size.height());
    QPixmap scaled = pix.scaled(QSize(targetWidth, targetHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap(centeredRect(center, scaled.size()), scaled, scaled.rect());
}

void drawScaledPixmapRotated(QPainter& painter, const QPixmap& pix, const QPointF& center, const QSizeF& size, double rotationDeg) {
    if (pix.isNull()) return;

    const int targetWidth = size.width();
    const int targetHeight = size.height();
    QPixmap scaled = pix.scaled(QSize(targetWidth, targetHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    painter.save();
    painter.translate(center);
    painter.rotate(rotationDeg);

    const QRectF target(-scaled.width() / 2.0, -scaled.height() / 2.0, scaled.width(), scaled.height());
    painter.drawPixmap(target, scaled, scaled.rect());
    painter.restore();
}

void PlayerPreviewWidget::drawWeapon(QPainter& painter, const QPointF& center, const QSizeF& size) {
    drawScaledPixmapRotated(painter, m_weaponSprite, center, size, 90);
}

void PlayerPreviewWidget::drawPlayerSkin(QPainter& painter, const QPointF& center, const QSizeF& size) {
    drawScaledPixmap(painter, m_skinSprite, center, size);
}
