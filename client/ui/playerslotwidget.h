#ifndef PLAYERSLOTWIDGET_H
#define PLAYERSLOTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QVector>

#include "../entities/player.h"
#include "../map/tilevisual.h"

class PlayerSlotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerSlotWidget(QWidget* parent = nullptr);

    void setPlayer(const Player*);

    void setSlotKeys(const QVector<Qt::Key>&);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    const Player* m_player = nullptr;

    QVector<Qt::Key> m_slotKeys;

    static constexpr unsigned short SLOT_SIZE = 50;
    static constexpr unsigned short SLOT_SPACING = 10;
    static constexpr unsigned short SLOT_COUNT = Inventory::MAX_SLOTS;

    void drawSlot(QPainter&, int, const Inventory&);
    void drawSlotIcon(QPainter&, const QRect&, const InventorySlot&);
    void drawSlotKey(QPainter&, int, int);
    void drawSlotCooldown(QPainter&, const QRect&, const InventorySlot&);
};

#endif // PLAYERSLOTWIDGET_H
