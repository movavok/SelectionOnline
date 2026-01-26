#ifndef PLAYERSLOTWIDGET_H
#define PLAYERSLOTWIDGET_H

#include <QWidget>
#include <QPainter>

#include "../entities/player.h"
#include "../map/tilevisual.h"

class PlayerSlotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerSlotWidget(QWidget* parent = nullptr);

    void setPlayer(const Player*);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    const Player* m_player = nullptr;

    static constexpr unsigned short SLOT_SIZE = 50;
    static constexpr unsigned short SLOT_SPACING = 10;
    static constexpr unsigned short SLOT_COUNT = Inventory::MAX_SLOTS;

    void drawSlot(QPainter&, int, const Inventory&);
    void drawSlotIcon(QPainter&, const QRect&, const InventorySlot&);
    void drawSlotKey(QPainter&, int, int);
};

#endif // PLAYERSLOTWIDGET_H
