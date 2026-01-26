#include "playerslotwidget.h"

PlayerSlotWidget::PlayerSlotWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(SLOT_SIZE + 25);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void PlayerSlotWidget::setPlayer(const Player* player) {
    m_player = player;
    update();
}

void PlayerSlotWidget::drawSlotIcon(QPainter& painter, const QRect& slotRect, const InventorySlot& slot) {
    int margin = slotRect.width() * 0.15;
    QRect iconRect = slotRect.adjusted(margin, margin, -margin, -margin);
    QPixmap icon;
    double opacity = 1.0;

    if (slot.m_type == InventorySlot::SlotType::Weapon && slot.m_weapon) {
        icon = slot.m_weapon->getIcon();
        opacity = 0.6;
    }
    else if (slot.m_type == InventorySlot::SlotType::Resource) {
        icon = tileVisual(slot.m_resourceType).sprite;
        opacity = 0.7;
    }

    if (!icon.isNull()) {
        painter.setOpacity(opacity);
        QPixmap scaled = icon.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(iconRect, scaled);
        painter.setOpacity(1.0);
    }

    if (slot.m_type == InventorySlot::SlotType::Resource) {
        painter.setFont(QFont("Fixedsys", 14));
        painter.setPen(Qt::white);
        painter.drawText(slotRect, Qt::AlignCenter, QString::number(slot.m_resourceAmount));
    }
}

void PlayerSlotWidget::drawSlotKey(QPainter& painter, int index, int width) {
    QRect keyRect(width, SLOT_SIZE + 2, SLOT_SIZE, 20);
    QFont keyFont("Fixedsys", 12, QFont::Bold);
    painter.setFont(keyFont);
    bool active = (index == m_player->getActiveSlot());
    painter.setPen(active ? QColor(255, 200, 80, 200) : QColor(255, 255, 255, 200));
    painter.drawText(keyRect, Qt::AlignCenter, QString::number(index + 1));
}

void PlayerSlotWidget::drawSlot(QPainter& painter, int index, const Inventory& inventory) {
    int width = index * (SLOT_SIZE + SLOT_SPACING);
    QRect slotRect(width, 0, SLOT_SIZE, SLOT_SIZE);

    bool active = (index == m_player->getActiveSlot());
    painter.setBrush(active ? QColor(255, 200, 80, 180) : QColor(30, 30, 30, 140));
    painter.setPen(QPen(QColor(200, 200, 200, 200), 3));
    painter.drawRect(slotRect);

    const InventorySlot& slot = inventory.getSlot(index);

    drawSlotIcon(painter, slotRect, slot);
    drawSlotKey(painter, index, width);
}

void PlayerSlotWidget::paintEvent(QPaintEvent*) {
    if (!m_player) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const Inventory& inventory = m_player->getInventory();

    for (int i = 0; i < SLOT_COUNT; ++i)
        drawSlot(painter, i, inventory);
}
