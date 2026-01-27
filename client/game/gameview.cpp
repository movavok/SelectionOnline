#include "gameview.h"

GameView::GameView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_timer(new QTimer(this))
    , m_gameTimer(new GameTimer(this))
{
    setScene(m_scene);
    m_scene->setSceneRect(m_game.getWorldBounds());
    buildMap();

    initDarkOverlay();
    initSlotWidget();

    initGameTimer();
    initTimerUi();

    initEntitiesUi();

    connect(m_timer, &QTimer::timeout, this, &GameView::onTick);
    m_timer->start(16); // ~60fps

    connect(&m_game, &Game::tileChanged, this, &GameView::updateTile);

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scale(m_scaleSize, m_scaleSize);
}

void GameView::initDarkOverlay() {
    m_darkOverlay = new QGraphicsRectItem();
    m_darkOverlay->setBrush(QColor(0, 0, 0));
    m_darkOverlay->setPen(Qt::NoPen);
    m_darkOverlay->setZValue(500);

    m_darkOverlay->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);

    m_scene->addItem(m_darkOverlay);
}

void GameView::startGameWithCountdown() {
    m_darkAmount = 1.0f;
    m_darkOverlay->setOpacity(m_darkAmount);
    m_darkMode = DarkOverlayMode::FadeOut;

    m_gameTimer->startCountdown(5);
}

void GameView::onCountdownTick(int sec) {
    sec = std::max(0, sec);
    m_timerLabel->setText(QString::number(sec));
    m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255, 200, 50);");
    m_timerLabel->setVisible(true);
}

void GameView::onGameStarted() {
    m_gamePaused = false;

    m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255, 255, 255);");
    m_gameTimer->startGameTimer(180);
}

void GameView::onGameTimerTick(int sec) {
    sec = std::max(0, sec);
    int minute = sec / 60;
    int second = sec % 60;

    m_timerLabel->setText(QString("%1:%2").arg(minute, 2, 10, QChar('0'))
                                          .arg(second, 2, 10, QChar('0')));
}

void GameView::onGameEnded() {
    m_gamePaused = true;
    m_darkMode = DarkOverlayMode::FadeIn;
    m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255, 20, 0);");
}

void GameView::initGameTimer() {
    connect(m_gameTimer, &GameTimer::countdownTick, this, &GameView::onCountdownTick);
    connect(m_gameTimer, &GameTimer::gameStarted, this, &GameView::onGameStarted);
    connect(m_gameTimer, &GameTimer::gameTick, this, &GameView::onGameTimerTick);
    connect(m_gameTimer, &GameTimer::gameEnded, this, &GameView::onGameEnded);
}

void GameView::initTimerUi() {
    m_timerLabel = new QLabel(this);

    m_timerBaseStyle = "background-color: rgba(0,0,0,150);"
                       "border: 2px solid rgb(200,200,200);"
                       "font: bold 14px 'Fixedsys';";

    m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255,220,50);");

    m_timerLabel->setAlignment(Qt::AlignCenter);
    m_timerLabel->setFixedSize(80, 30);
    m_timerLabel->move((viewport()->width() - m_timerLabel->width()) / 2, 20);
    m_timerLabel->show();
}

void GameView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);

    if (m_slotWidget)
        m_slotWidget->move((viewport()->width() - m_slotWidget->width()) / 2,
                           viewport()->height() - m_slotWidget->height() * 1.5);

    if (m_timerLabel)
        m_timerLabel->move((viewport()->width() - m_timerLabel->width()) / 2, 20);
}

void GameView::initSlotWidget() {
    m_slotWidget = new PlayerSlotWidget(this);
    m_slotWidget->setFixedSize(300, 70);

    m_slotWidget->raise();
    m_slotWidget->show();

    if (Player* player = m_game.getPlayer())
        m_slotWidget->setPlayer(player);
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

void GameView::initSlotIndicator(EntityUi& ui) {
    ui.slotIndicator = new QGraphicsPixmapItem(ui.body);
    ui.slotIndicator->setZValue(6);
    ui.slotIndicator->hide();
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
        initSlotIndicator(ui);

        m_entityItems.insert(entity, ui);
    }
}

void GameView::initBuildPreview() {
    m_buildPreview = new QGraphicsPixmapItem();
    m_buildPreview->setOpacity(0.5);
    m_buildPreview->setZValue(3);
    m_buildPreview->hide();
    m_scene->addItem(m_buildPreview);
}

void GameView::buildMap() {
    const Map& map = m_game.getMap();

    int mapWidth = map.getTileCountX() * Map::TILE_SIZE;
    int mapHeight = map.getTileCountY() * Map::TILE_SIZE;
    double offsetX = -mapWidth / 2.0;
    double offsetY = -mapHeight / 2.0;

    initBuildPreview();

    for (int y = 0; y < map.getTileCountY(); ++y) {
        for (int x = 0; x < map.getTileCountX(); ++x) {

            QGraphicsPixmapItem* tileItem = new QGraphicsPixmapItem();
            tileItem->setPos(x * Map::TILE_SIZE + offsetX, y * Map::TILE_SIZE + offsetY);
            tileItem->setZValue(0);

            const Tile& tile = map.tileAt(x, y);
            const TileVisual& visual = tile.getType() == Tile::TileType::Empty ? emptyVisual(tile.getVariation())
                                                                               : tileVisual(tile.getType());
            if (!visual.sprite.isNull())
                tileItem->setPixmap(visual.sprite.scaled(Map::TILE_SIZE, Map::TILE_SIZE,
                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

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
    if (m_moveKeyMap.contains(scanCode))
        m_game.setPlayerInput(m_moveKeyMap[scanCode], pressed);
    if (pressed && m_slotKeyMap.contains(scanCode))
        m_game.getPlayer()->setActiveSlot(m_slotKeyMap[scanCode]);
}

void GameView::keyPressEvent(QKeyEvent* event) { handleKeyEvent(event, true); }
void GameView::keyReleaseEvent(QKeyEvent* event) { handleKeyEvent(event, false); }

void GameView::mousePressEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) m_game.getPlayer()->startAiming();
}

void GameView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;

    Player* player = m_game.getPlayer();
    if (!player) return;

    QPointF mouseScene = mapToScene(event->pos());

    if (player->getInventory().isActiveResource())
        m_game.tryPlaceTile(m_game.getMap().worldToTile(mouseScene));
    else
        player->stopAiming(mouseScene - player->getPosition());
}

void GameView::wheelEvent(QWheelEvent* event) { event->ignore(); }

void GameView::setupSlotKeys() {
    m_slotKeyMap[2] = 0;
    m_slotKeyMap[3] = 1;
    m_slotKeyMap[4] = 2;
    m_slotKeyMap[5] = 3;
    m_slotKeyMap[6] = 4;
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

    m_moveKeyMap.clear();
    m_moveKeyMap[scanUp] = MoveDirection::MoveUp;
    m_moveKeyMap[scanDown] = MoveDirection::MoveDown;
    m_moveKeyMap[scanLeft] = MoveDirection::MoveLeft;
    m_moveKeyMap[scanRight] = MoveDirection::MoveRight;
}

void GameView::updateDarkOverlayRect() {
    QPointF topLeft = mapToScene(QPoint(0,0));
    QPointF bottomRight = mapToScene(QPoint(viewport()->width(), viewport()->height()));

    QRectF rect(topLeft, bottomRight);
    m_darkOverlay->setRect(rect.normalized());
}

void GameView::updateDarkOverlay() {
    if (m_darkMode == DarkOverlayMode::FadeOut) {
        m_darkAmount -= m_deltaTime * 0.25f;
        if (m_darkAmount <= 0.0f) {
            m_darkAmount = 0.0f;
            m_darkMode = DarkOverlayMode::None;
        }
    }
    else if (m_darkMode == DarkOverlayMode::FadeIn) {
        m_darkAmount += m_deltaTime * 0.25f;
        if (m_darkAmount >= 1.0f) {
            m_darkAmount = 1.0f;
            m_darkMode = DarkOverlayMode::None;
        }
    }

    m_darkOverlay->setOpacity(m_darkAmount);
    updateDarkOverlayRect();
}

void GameView::updateCamera() {
    if (Player* player = m_game.getPlayer()) {
        const QPointF pos = player->getPosition();

        m_entityItems[player].body->setPos(pos - QPointF(player->getRadius(), player->getRadius()));

        m_cameraPos = m_cameraPos * 0.95 + pos * 0.05;
        centerOn(m_cameraPos);
    }
}

void GameView::updateSlotWidget() {
    if (m_slotWidget && m_game.getPlayer())
        m_slotWidget->update();
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
        const Weapon* weapon = player->getInventory().getActiveWeapon();
        if (!weapon || player->getAttackState() == Player::AttackState::Idle) {
            ui.attackIndicator->hide();
            return;
        }

        ui.attackIndicator->setBrush(player->canAttack() ? QColor(255, 220, 100, 120)
                                                         : QColor(150, 150, 150, 120));

        QPointF mouseScene = mapToScene(mapFromGlobal(QCursor::pos()));
        QPainterPath cuttedShape = m_game.getPlayerAttackShape(mouseScene);

        QPointF center(player->getRadius() / 2, player->getRadius() / 2);
        QTransform toLocal;
        toLocal.translate(-player->getPosition().x() + center.x(),
                          -player->getPosition().y() + center.y());

        ui.attackIndicator->setPath(toLocal.map(cuttedShape));
        ui.attackIndicator->setPos(center);
        ui.attackIndicator->show();
    }
}

void GameView::updateEntitiesVisibility(Entity* entity, EntityUi& ui) {
    const float radius = entity->getRadius();

    QRectF entityRect(entity->getPosition() - QPointF(radius, radius), QSizeF(radius * 2, radius * 2));

    bool inGrass = m_game.getMap().intersectsGrass(entityRect);

    if (entity == m_game.getPlayer()) {
        ui.body->setVisible(true);
        ui.body->setOpacity(inGrass ? 0.5 : 1.0);
    } else {
        ui.body->setOpacity(1.0);
        ui.body->setVisible(!inGrass);
    }
}

void GameView::updateEntitySlotIndicator(Entity* entity, EntityUi& ui) {
    Player* player = dynamic_cast<Player*>(entity);
    if (!player) return;

    QPixmap pixmap;

    if (player->getInventory().isActiveResource()) {
        Tile::TileType type = player->getInventory().getActiveResourceType();
        const TileVisual& visual = tileVisual(type);
        if (!visual.sprite.isNull())
            pixmap = visual.sprite.scaled(10, 10, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else if (Weapon* weapon = player->getInventory().getActiveWeapon())
        pixmap = weapon->getSprite().scaled(weapon->getSpriteSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (pixmap.isNull()) { ui.slotIndicator->hide(); return; }

    ui.slotIndicator->setPixmap(pixmap);
    ui.slotIndicator->setOffset(-pixmap.width() / 2.0, -pixmap.height() / 2.0);
    ui.slotIndicator->setTransformOriginPoint(0, 0);
    ui.slotIndicator->show();

    QPointF mouseScene = mapToScene(mapFromGlobal(QCursor::pos()));
    QPointF playerCenterScene = ui.body->sceneBoundingRect().center();

    QPointF dir = mouseScene - playerCenterScene;
    double angle = std::atan2(dir.y(), dir.x());

    double orbitRadius = player->getRadius() + 2.0;

    QPointF localPos(std::cos(angle) * orbitRadius, std::sin(angle) * orbitRadius);
    QPointF bodyCenter(player->getRadius(), player->getRadius());
    ui.slotIndicator->setPos(bodyCenter + localPos);

    ui.slotIndicator->setRotation(qRadiansToDegrees(angle));
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

        updateEntitiesVisibility(entity, ui);
        updateEntityHp(entity, ui);
        updateEntityAtkIndicator(entity, ui);
        updateEntitySlotIndicator(entity, ui);
    }
}

void GameView::updateTile(int x, int y) {
    QPoint key(x, y);
    if (m_tileItems.contains(key)){
        const Tile& tile = m_game.getMap().tileAt(x, y);
        const TileVisual& visual = tile.getType() == Tile::TileType::Empty ? emptyVisual(tile.getVariation())
                                                                           : tileVisual(tile.getType());

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

void GameView::updateBuildPreview() {
    Player* player = m_game.getPlayer();
    if (!player || !player->getInventory().isActiveResource()) {
        m_buildPreview->hide();
        return;
    }

    QPointF mouseScene = mapToScene(mapFromGlobal(QCursor::pos()));
    QPoint tile = m_game.getMap().worldToTile(mouseScene);
    Tile::TileType type = player->getInventory().getActiveResourceType();

    if (tile == m_previewTile && type == m_previewType) return;

    m_previewTile = tile;
    m_previewType = type;

    if (!m_game.canPlaceTile(tile)) {
        m_buildPreview->hide();
        return;
    }

    const TileVisual& visual = tileVisual(type);
    if (visual.sprite.isNull()) {
        m_buildPreview->hide();
        return;
    }

    m_buildPreview->setPixmap(visual.sprite.scaled(Map::TILE_SIZE, Map::TILE_SIZE,
                                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    QPointF world = m_game.getMap().tileToWorld(tile);
    m_buildPreview->setPos(world - QPointF(Map::TILE_SIZE / 2, Map::TILE_SIZE / 2));

    m_buildPreview->show();
}

void GameView::onTick() {
    m_gameTimer->update(m_deltaTime);

    if (!m_gamePaused) m_game.update(m_deltaTime);

    updateDarkOverlay();

    updateCamera();

    updateSlotWidget();
    updateEntitiesUi();
    updatePickupsUi();
    updateBuildPreview();
}
