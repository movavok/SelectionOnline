#include "gameview.h"

GameView::GameView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_timer(new QTimer(this))
{
    setScene(m_scene);

    initEntitiesUi();

    m_scene->setSceneRect(m_game.getWorldBounds());
    buildMap();

    connect(m_timer, &QTimer::timeout, this, &GameView::onTick);
    m_timer->start(16); // ~60fps

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
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

void GameView::initHpBar(Entity* entity, EntityUi& ui) {
    double width = entity->getRadius() * 2;
    double height = 10;
    QPointF pos(0, -height - 6);

    ui.hpBack = createRectItem(ui.body, QRectF(0, 0, width, height), QColor(50,50,50), 10);
    ui.hpBack->setPos(pos);

    ui.hpFill = createRectItem(ui.hpBack, QRectF(0, 0, width, height), QColor(60,220,80), 11);
    ui.hpFill->setPos(0, 0);

    ui.hpTextMask = createRectItem(ui.hpBack, QRectF(0, 0, width, height), Qt::transparent, 13);
    ui.hpTextMask->setPos(0, 0);
    ui.hpTextMask->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    ui.hpTextWhite = createTextItem(ui.hpBack, Qt::white, 12);
    ui.hpTextBlack = createTextItem(ui.hpTextMask, Qt::black, 14);
}

void GameView::initAttackIndicator(EntityUi& ui) {
    ui.attackIndicator = new QGraphicsPathItem(ui.body);
    ui.attackIndicator->setBrush(QColor(150, 150, 150, 120));
    ui.attackIndicator->setPen(Qt::NoPen);
    ui.attackIndicator->hide();
}

void GameView::initEntitiesUi() {
    for (Entity* entity : m_game.getEntities()) {
        EntityUi ui;
        ui.body = new QGraphicsEllipseItem(0, 0, entity->getRadius() * 2, entity->getRadius() * 2);
        ui.body->setBrush(dynamic_cast<Player*>(entity) ? Qt::blue : Qt::red);
        ui.body->setPen(Qt::NoPen);
        ui.body->setZValue(5);
        m_scene->addItem(ui.body);

        ui.body->setPos(entity->getPosition() - QPointF(entity->getRadius(), entity->getRadius()));

        initHpBar(entity, ui);
        initAttackIndicator(ui);

        m_entityItems.insert(entity, ui);
    }
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

void GameView::mousePressEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) m_game.getPlayer()->startAiming();
}

void GameView::mouseReleaseEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton)
        m_game.getPlayer()->stopAiming(m_mouseScenePos - m_game.getPlayer()->getPosition());
}

void GameView::mouseMoveEvent(QMouseEvent* event) { m_mouseScenePos = mapToScene(event->pos()); }

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
        const QPointF pos = player->getPosition();

        m_entityItems[player].body->setPos(pos - QPointF(player->getRadius(), player->getRadius()));

        m_cameraPos = m_cameraPos * 0.95 + pos * 0.05;
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

void GameView::updateEntityHp(Entity* entity, EntityUi& ui) {
    const double ratio = double(entity->getCurrentHp()) / entity->getMaxHp();
    const QRectF backRect = ui.hpBack->rect();
    ui.hpFill->setRect(0, 0, backRect.width() * ratio, backRect.height());
    ui.hpFill->setBrush(getHpBarColor(ratio));

    const QString text = QString::number(entity->getCurrentHp());
    ui.hpTextWhite->setPlainText(text);
    ui.hpTextBlack->setPlainText(text);

    const QRectF textRect = ui.hpTextWhite->boundingRect();
    const double fillWidth = backRect.width() * ratio;

    ui.hpTextMask->setRect(0, 0, fillWidth, backRect.height());
    ui.hpTextWhite->setPos(backRect.width()/2 - textRect.width()/2,
                               backRect.height()/2 - textRect.height()/2);
    ui.hpTextBlack->setPos(ui.hpTextWhite->pos());
}

void GameView::updateEntityAtkIndicator(Entity* entity, EntityUi& ui) {
    if (Player* player = dynamic_cast<Player*>(entity)) {
        if (player->getAttackState() == Player::AttackState::Idle) {
            ui.attackIndicator->hide();
            return;
        }

        ui.attackIndicator->setPos(player->getRadius(), player->getRadius());
        ui.attackIndicator->setPath(player->getWeapon()->indicatorShape(*player));

        const QPointF dir = m_mouseScenePos - player->getPosition();
        const double angleDeg = qRadiansToDegrees(std::atan2(dir.y(), dir.x()));
        ui.attackIndicator->setRotation(angleDeg);
        ui.attackIndicator->show();
    }
}

void GameView::updateEntitiesUi() {
    for (Entity* entity : m_game.getEntities()) {
        EntityUi& ui = m_entityItems[entity];

        if (!entity->isAlive()) {
            ui.body->hide();
            continue;
        }

        ui.body->show();
        const QPointF pos = entity->getPosition();
        ui.body->setPos(pos - QPointF(entity->getRadius(), entity->getRadius()));

        updateEntityHp(entity, ui);
        updateEntityAtkIndicator(entity, ui);
    }
}

void GameView::onTick() {
    m_game.update(deltaTime);
    updateCamera();
    updateEntitiesUi();
}
