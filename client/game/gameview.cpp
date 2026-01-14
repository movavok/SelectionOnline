#include "gameview.h"

GameView::GameView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_playerItem(new QGraphicsPixmapItem())
    , m_timer(new QTimer(this))
{
    setScene(m_scene);

    QPixmap playerPx(30, 30); // temp drawing
    playerPx.fill(Qt::blue);

    m_playerItem->setPixmap(playerPx);
    m_playerItem->setOffset(0, 0);

    m_scene->setSceneRect(m_game.getWorldBounds());
    buildMap();
    m_scene->addItem(m_playerItem);

    connect(m_timer, &QTimer::timeout, this, &GameView::onTick);
    m_timer->start(16); // ~60fps

    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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

            tileItem->setData(0, visual.solid);

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

void GameView::onTick() {
    m_game.update(deltaTime);

    if (Player* player = m_game.getPlayer()) {
        m_playerItem->setPos(player->getPosition());
        centerOn(m_playerItem);
    }
}
