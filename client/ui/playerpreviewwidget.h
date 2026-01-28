#ifndef PLAYERPREVIEWWIDGET_H
#define PLAYERPREVIEWWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QSizePolicy>
#include <QString>

class PlayerPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    enum class WeaponType {
        Empty,
        Katana
    };

    enum class AbilityType {
        Empty,
        MirrorShield
    };

    explicit PlayerPreviewWidget(QWidget* parent = nullptr);

    void setNickname(const QString&);
    void setColor(const QColor&);
    void setWeapon(WeaponType);
    void setAbility(AbilityType);

protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;

private:
    QString m_nickname;

    QColor m_color = Qt::transparent;
    WeaponType m_weaponType = WeaponType::Empty;
    AbilityType m_abilityType = AbilityType::Empty;

    QPixmap m_weaponSprite;
    QPixmap m_skinSprite;
    QPixmap m_background;

    void drawColoredCircle(QPainter&, const QPointF&, const QSizeF&);
    void drawWeapon(QPainter&, const QPointF&, const QSizeF&);
    void drawPlayerSkin(QPainter&, const QPointF&, const QSizeF&);
};

#endif // PLAYERPREVIEWWIDGET_H
