#include "gameview.h"

GameView::GameView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_timer(new QTimer(this))
    , m_playerItem(new QGraphicsPixmapItem())
{
    setScene(m_scene);

    QPixmap playerPx(30, 30); // temp drawing
    playerPx.fill(Qt::blue);

    m_playerItem->setPixmap(playerPx);

    initHpBar();

    m_scene->setSceneRect(m_game.getWorldBounds());
    buildMap();
    m_scene->addItem(m_playerItem);

    connect(m_timer, &QTimer::timeout, this, &GameView::onTick);
    m_timer->start(16); // ~60fps

    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

QGraphicsRectItem* createRectItem(QGraphicsItem* parent, const QRectF& rect, const QColor& color, int zValue) {
    QGraphicsRectItem* item = new QGraphicsRectItem(rect, parent);
    item->setBrush(color);
    item->setPen(Qt::NoPen);
    item->setZValue(zValue);
    item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    return item;
}

QGraphicsTextItem* createTextItem(QGraphicsItem* parent, const QColor& color, int zValue, int fontSize = 8) {
    QGraphicsTextItem* item = new QGraphicsTextItem(parent);
    item->setDefaultTextColor(color);
    item->setFont(QFont("Arial", fontSize, QFont::Bold));
    item->setZValue(zValue);
    item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    return item;
}

void GameView::initHpBar() {
    const double barWidth = m_game.getPlayer()->getWidth();
    const double barHeight = 10;
    const double gap = 6;

    const QPointF pos(0, -barHeight - gap);

    m_hpBack = createRectItem(m_playerItem, QRectF(0, 0, barWidth, barHeight), QColor(50, 50, 50), 10);
    m_hpBack->setPos(pos);

    m_hpFill = createRectItem(m_playerItem, QRectF(0, 0, barWidth, barHeight), QColor(60, 220, 80), 11);
    m_hpFill->setPos(pos);

    m_hpTextMask = createRectItem(m_playerItem, QRectF(0, 0, barWidth, barHeight), Qt::transparent, 13);
    m_hpTextMask->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    m_hpTextMask->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    m_hpTextWhite = createTextItem(m_playerItem, Qt::white, 12);
    m_hpTextBlack = createTextItem(m_hpTextMask, Qt::black, 14);
}

void GameView::buildMap() {
    const Map& map = m_game.getMap();

    int mapWidth = map.getTileCountX() * Map::TILE_SIZE;
    int mapHeight = map.getTileCountY() * Map::TILE_SIZE;
    double offsetX = -mapWidth / 2.0;
    double offsetY = -mapHeight / 2.0;

    for (int y = 0; y < map.getTileCountY(); ++y) {
        for (int x = 0; x < map.getTileCountX(); ++x) {
            const TileVisual& visual = tileVisual(map.tileAt(x, y).getType());
            if (visual.color == Qt::transparent) continue;

            QRectF rect(x * Map::TILE_SIZE + offsetX, y * Map::TILE_SIZE + offsetY,
                        Map::TILE_SIZE, Map::TILE_SIZE);

            QGraphicsRectItem* tileItem = new QGraphicsRectItem(rect);
            tileItem->setBrush(visual.color);
            tileItem->setPen(Qt::NoPen);

            m_scene->addItem(tileItem);
        }
    }
}

void GameView::handleKeyEvent(QKeyEvent* event, bool pressed) {
    if (event->isAutoRepeat()) return;
    Qt::Key key = static_cast<Qt::Key>(event->key());
    if (m_keyMap.contains(key))
        m_game.setPlayerInput(m_keyMap[key], pressed);
}

void GameView::keyPressEvent(QKeyEvent* event) { handleKeyEvent(event, true); }
void GameView::keyReleaseEvent(QKeyEvent* event) { handleKeyEvent(event, false); }

void GameView::mousePressEvent(QMouseEvent*) {}

void GameView::useMovementScheme(MovementScheme scheme) {
    Qt::Key keyUp, keyDown, keyLeft, keyRight;

    if (scheme == MovementScheme::WASD) { keyUp = Qt::Key_W; keyDown = Qt::Key_S; keyLeft = Qt::Key_A; keyRight = Qt::Key_D; }
    else if (scheme == MovementScheme::Arrows) { keyUp = Qt::Key_Up; keyDown = Qt::Key_Down; keyLeft = Qt::Key_Left; keyRight = Qt::Key_Right; }

    m_keyMap.clear();
    m_keyMap[keyUp] = MoveDirection::MoveUp;
    m_keyMap[keyDown] = MoveDirection::MoveDown;
    m_keyMap[keyLeft] = MoveDirection::MoveLeft;
    m_keyMap[keyRight] = MoveDirection::MoveRight;
}

void GameView::updateCamera() {
    if (Player* player = m_game.getPlayer()) {
        m_playerItem->setPos(player->getPosition() - QPointF(player->getWidth() / 2, player->getHeight() / 2));
        m_cameraPos = m_cameraPos * 0.95 + player->getPosition() * 0.05;
        centerOn(m_cameraPos);
    }
}

QColor getHpBarColor(double ratio) {
    unsigned short red, green;
    double progress;

    if (ratio > 0.75) {
        progress = (ratio - 0.75) / 0.25;
        red = int(60   + (255 - 60) * (1.0 - progress));
        green = 255;
    } else if (ratio > 0.5) {
        progress = (ratio - 0.5) / 0.25;
        red = 255;
        green = int(165 + (255 - 165) * progress);
    } else {
        progress = ratio / 0.5;
        red = 255;
        green = int(165 * progress);
    }

    return QColor(red, green, 0);
}

void GameView::updateHpBar() {
    Player* player = m_game.getPlayer();
    if (!player || !m_hpFill) return;

    const int hp = player->getCurrentHp();
    const int maxHp = player->getMaxHp();

    const double ratio = maxHp > 0 ? std::clamp(double(hp) / maxHp, 0.0, 1.0) : 0.0;

    const QRectF backRect = m_hpBack->rect();
    m_hpFill->setRect(0, 0, backRect.width() * ratio, backRect.height());
    m_hpFill->setBrush(getHpBarColor(ratio));

    const double fillWidth = backRect.width() * ratio;
    const QPointF barPos = m_hpBack->pos();

    m_hpTextMask->setPos(barPos);
    m_hpTextMask->setRect(0, 0, fillWidth, backRect.height());

    const QString text = QString::number(hp);
    m_hpTextWhite->setPlainText(text);
    m_hpTextBlack->setPlainText(text);

    const QRectF textRect = m_hpTextWhite->boundingRect();
    const QPointF centeredPos = barPos + QPointF(backRect.width() / 2 - textRect.width() / 2,
                                                 backRect.height() / 2 - textRect.height() / 2);

    m_hpTextWhite->setPos(centeredPos);
    m_hpTextBlack->setPos(centeredPos - m_hpTextMask->pos());
}

void GameView::onTick() {
    m_game.update(deltaTime);
    updateCamera();
    updateHpBar();
    m_game.getPlayer()->setCurrentHp(m_game.getPlayer()->getCurrentHp() - 1);
    if (m_game.getPlayer()->getCurrentHp() <= 0) m_game.getPlayer()->setCurrentHp(m_game.getPlayer()->getMaxHp());
}
