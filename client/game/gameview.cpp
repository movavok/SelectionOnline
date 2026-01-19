#include "gameview.h"

GameView::GameView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_timer(new QTimer(this))
{
    srand(static_cast<unsigned>(time(nullptr)));

    setScene(m_scene);

    initEntitiesUi();

    m_scene->setSceneRect(m_game.getWorldBounds());
    buildMap();

    connect(m_timer, &QTimer::timeout, this, &GameView::onTick);
    m_timer->start(16); // ~60fps

    connect(&m_game, &Game::tileChanged, this, &GameView::updateTile);

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scale(m_scaleSize, m_scaleSize);
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

void GameView::initHpBar(EntityUi& ui) {
    double width = ui.body->boundingRect().width() * m_scaleSize;
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

        const float radius = entity->getRadius();
        ui.body = new QGraphicsEllipseItem(0, 0, entity->getRadius() * 2, entity->getRadius() * 2);
        ui.body->setBrush(dynamic_cast<Player*>(entity) ? QColor(0, 200, 255, 150) : QColor(255, 50, 50, 150));
        ui.body->setPen(Qt::NoPen);
        ui.body->setZValue(5);
        m_scene->addItem(ui.body);

        QPixmap sprite;
        sprite.load(":/entities/tank.png");

        ui.sprite = new QGraphicsPixmapItem(sprite.scaled(radius * 2, radius * 2), ui.body);
        ui.sprite->setZValue(5);
        ui.sprite->setTransformOriginPoint(ui.sprite->boundingRect().center());

        ui.body->setPos(entity->getPosition());

        initHpBar(ui);
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
            if (visual.sprite.isNull()) continue;

            QGraphicsPixmapItem* tileItem = new QGraphicsPixmapItem(visual.sprite.scaled(Map::TILE_SIZE, Map::TILE_SIZE,
                                                                                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            tileItem->setPos(x * Map::TILE_SIZE + offsetX, y * Map::TILE_SIZE + offsetY);
            tileItem->setZValue(0);

            m_scene->addItem(tileItem);
            m_tileItems[QPoint(x, y)] = tileItem;
        }
    }
}

Tile::TileType displayPickupType(const PickupItem* pickup) {
    switch (pickup->getType()) {
    case Tile::TileType::BrickCracked: return Tile::TileType::BrickStrong;
    default: return pickup->getType();
    }
}

void GameView::createPickupUi(const PickupItem* pickup) {
    const TileVisual& visual = tileVisual(displayPickupType(pickup));
    if (visual.sprite.isNull()) return;

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(visual.sprite);

    const double targetSize = Map::TILE_SIZE * 0.5;
    double scale = targetSize / visual.sprite.width();
    item->setScale(scale);

    item->setOffset(-visual.sprite.width() / 2.0, -visual.sprite.height() / 2.0);

    item->setPos(pickup->getPosition());
    item->setZValue(2);

    item->setRotation(rand() % 41 - 20);
    item->setScale(item->scale() * (0.9 + (rand() % 21) / 100.0));

    m_scene->addItem(item);
    m_pickupItems.insert(pickup, item);
}

void GameView::handleKeyEvent(QKeyEvent* event, bool pressed) {
    if (event->isAutoRepeat()) return;
    unsigned short scanCode = static_cast<unsigned short>(event->nativeScanCode());
    if (m_keyMap.contains(scanCode))
        m_game.setPlayerInput(m_keyMap[scanCode], pressed);
}

void GameView::keyPressEvent(QKeyEvent* event) { handleKeyEvent(event, true); }
void GameView::keyReleaseEvent(QKeyEvent* event) { handleKeyEvent(event, false); }

void GameView::mousePressEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) m_game.getPlayer()->startAiming();
}

void GameView::mouseReleaseEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) {
        QPointF mouseScene = mapToScene(mapFromGlobal(QCursor::pos()));
        m_game.getPlayer()->stopAiming(mouseScene - m_game.getPlayer()->getPosition());
    }
}

void GameView::useMovementScheme(MovementScheme scheme) {
    unsigned short scanUp, scanDown, scanLeft, scanRight;

    if (scheme == MovementScheme::WASD) {
        scanUp = 17; // W
        scanDown = 31; // S
        scanLeft = 30; // A
        scanRight = 32; // D
    }
    else if (scheme == MovementScheme::Arrows) {
        scanUp = 72; // Up
        scanDown = 80; // Down
        scanLeft = 75; // Left
        scanRight = 77; // Right
    }

    m_keyMap.clear();
    m_keyMap[scanUp] = MoveDirection::MoveUp;
    m_keyMap[scanDown] = MoveDirection::MoveDown;
    m_keyMap[scanLeft] = MoveDirection::MoveLeft;
    m_keyMap[scanRight] = MoveDirection::MoveRight;
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

        QPointF mouseScene = mapToScene(mapFromGlobal(QCursor::pos()));

        ui.attackIndicator->setPos(player->getRadius(), player->getRadius());
        ui.attackIndicator->setPath(player->getInventory().getActiveWeapon()->indicatorShape(*player));

        const QPointF dir = mouseScene - player->getPosition();
        ui.attackIndicator->setRotation(qRadiansToDegrees(std::atan2(dir.y(), dir.x())));
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

        QPointF dir = entity->getPosition() - entity->getPrevPosition();
        if (!dir.isNull())
            m_entityItems[entity].sprite->setRotation(qRadiansToDegrees(std::atan2(dir.y(), dir.x())) - 90);

        updateEntityHp(entity, ui);
        updateEntityAtkIndicator(entity, ui);
    }
}

void GameView::updateTile(int x, int y) {
    QPoint key(x, y);
    if (m_tileItems.contains(key)){
        const TileVisual& visual = tileVisual(m_game.getMap().tileAt(x, y).getType());

        if (visual.sprite.isNull()) {
            m_tileItems[key]->setPixmap(QPixmap());
            return;
        }

        m_tileItems[key]->setPixmap(visual.sprite.scaled(Map::TILE_SIZE, Map::TILE_SIZE,
                                                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}

void GameView::updatePickupsUi() {
    for (const PickupItem* pickup : m_game.getPickups())
        if (!m_pickupItems.contains(pickup)) createPickupUi(pickup);

    for (auto iter = m_pickupItems.begin(); iter != m_pickupItems.end(); ) {
        if (!m_game.getPickups().contains(iter.key())) {
            delete iter.value();
            iter = m_pickupItems.erase(iter);
        } else ++iter;
    }
}

void GameView::onTick() {
    m_game.update(m_deltaTime);
    updateCamera();
    updateEntitiesUi();
    updatePickupsUi();
}
