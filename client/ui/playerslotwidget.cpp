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
    short margin;
    QPixmap icon;
    double opacity = 1.0;

    if (slot.m_type == InventorySlot::SlotType::Weapon && slot.m_weapon) {
        icon = slot.m_weapon->getIcon();
        margin = 0;
    }
    else if (slot.m_type == InventorySlot::SlotType::Resource) {
        icon = tileVisual(slot.m_resourceType).sprite;
        opacity = 0.7;
        margin = slotRect.width() * 0.2;
    }

    QRect iconRect = slotRect.adjusted(margin, margin, -margin, -margin);

    if (!icon.isNull()) {
        painter.setOpacity(opacity);
        QPixmap scaled = icon.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::FastTransformation);
        painter.drawPixmap(iconRect, scaled);
        painter.setOpacity(1.0);
    }

    if (slot.m_type == InventorySlot::SlotType::Resource && slot.m_resourceAmount > 1) {
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

void PlayerSlotWidget::drawSlotCooldown(QPainter& painter, const QRect& slotRect, const InventorySlot& slot) {
    if (!m_player) return;
    if (slot.m_type != InventorySlot::SlotType::Weapon) return;
    if (!slot.m_weapon) return;

    const Weapon* activeWeapon = m_player->getActiveWeapon();
    if (!activeWeapon) return;
    if (slot.m_weapon != activeWeapon) return;

    const float remaining = m_player->getAttackCooldownRemaining();
    const float total = m_player->getAttackCooldownTotal();
    if (remaining <= 0.0001f) return;
    if (total <= 0.0001f) return;

    // Progress grows upward while reloading.
    float progress = 1.0f - (remaining / total);
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    const int h = int(slotRect.height() * progress);
    if (h <= 0) return;

    QRect overlay(slotRect.left(), slotRect.bottom() - h + 1, slotRect.width(), h);
    painter.fillRect(overlay, QColor(120, 120, 120, 130));
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
    drawSlotCooldown(painter, slotRect, slot);
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
