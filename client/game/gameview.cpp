#include "gameview.h"

GameView::GameView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_timer(new QTimer(this))
    , m_gameTimer(new GameTimer(this))
{
    setScene(m_scene);
    m_scene->setSceneRect(m_fakeServer.game().getWorldBounds());
    buildMap();

    initDarkOverlay();
    initSlotWidget();
    initMiniMapWidget();

    initGameTimer();
    initTimerUi();

    initEntitiesUi();

    connect(m_timer, &QTimer::timeout, this, &GameView::onTick);
    m_timer->start(16); // ~60fps

    connect(&m_fakeServer.game(), &Game::tileChanged, this, &GameView::updateTile);
    connect(&m_fakeServer.game(), &Game::enemyHit, this, &GameView::onEnemyHit);
    connect(&m_fakeServer.game(), &Game::externalPlayerHit, this, &GameView::onExternalPlayerHit);
    connect(&m_fakeServer.game(), &Game::pickupCollected, this, &GameView::localPickupCollected);
    connect(&m_fakeServer.game(), &Game::localPlayerAttackPerformed, this, &GameView::onLocalPlayerAttackPerformed);

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scale(m_scaleSize, m_scaleSize);
}

quint32 GameView::remoteEntityIdFromPlayerId(quint32 playerId) {
    return 0x80000000u | playerId;
}

QColor GameView::uiColorFromColorId(quint8 colorId) {
    switch (colorId) {
    case 0: return QColor(Qt::red);
    case 1: return QColor(Qt::blue);
    case 2: return QColor(Qt::yellow);
    case 3: return QColor(Qt::green);
    case 4: return QColor(Qt::cyan);
    case 5: return QColor(Qt::white);
    case 6: return QColor(Qt::black);
    case 7: return QColor(128, 0, 128);
    case 8: return QColor(255, 105, 180);
    case 9: return QColor(255, 165, 0);
    default: return QColor();
    }
}

QPixmap GameView::buildKatanaSlotIndicatorPixmap() {
    static const QPixmap cached = QPixmap(":/weapons/katana.png")
                                     .scaled(QSize(6, 36), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return cached;
}

QPixmap GameView::buildResourceSlotIndicatorPixmap(quint8 resourceTileType) {
    const Tile::TileType tileType = static_cast<Tile::TileType>(resourceTileType);
    const TileVisual& visual = tileVisual(tileType);
    if (visual.sprite.isNull())
        return {};
    return visual.sprite.scaled(10, 10, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

float GameView::getRemotePlayerRadius() const {
    const Player* player = m_fakeServer.game().getPlayer();
    return player->getRadius();
}

void GameView::ensureLocalPlayerUi() {
    Player* player = m_fakeServer.game().getPlayer();
    if (!player) return;

    const quint32 localEntityId = m_fakeServer.entityIdFor(player);
    if (localEntityId == 0 || m_entityItems.contains(localEntityId))
        return;

    EntitySnapshot snapshotEntity;
    snapshotEntity.id = localEntityId;
    snapshotEntity.kind = SnapshotEntityKind::Player;
    snapshotEntity.alive = player->isAlive();
    snapshotEntity.pos = player->getPosition();
    snapshotEntity.prevPos = player->getPrevPosition();
    snapshotEntity.radius = player->getRadius();
    snapshotEntity.hp = player->getCurrentHp();
    snapshotEntity.maxHp = player->getMaxHp();
    snapshotEntity.nickname = player->getNickname();
    snapshotEntity.uiColor = player->getUiColor();
    snapshotEntity.isLocalPlayer = true;
    ensureEntityUi(snapshotEntity);
}

void GameView::ensureRemotePlayerUi(quint32 playerId) {
    if (playerId == 0) return;

    const quint32 remoteEntityId = remoteEntityIdFromPlayerId(playerId);
    if (remoteEntityId == 0 || m_entityItems.contains(remoteEntityId))
        return;

    const auto remoteIter = m_remotePlayers.constFind(playerId);
    if (remoteIter == m_remotePlayers.constEnd() || !remoteIter.value().initialized)
        return;

    EntitySnapshot snapshotEntity;
    snapshotEntity.id = remoteEntityId;
    snapshotEntity.kind = SnapshotEntityKind::Player;
    snapshotEntity.alive = true;
    snapshotEntity.pos = remoteIter.value().pos;
    snapshotEntity.prevPos = remoteIter.value().prevPos;
    snapshotEntity.aimDir = remoteIter.value().aimDir;
    snapshotEntity.radius = getRemotePlayerRadius();
    snapshotEntity.hp = remoteIter.value().hp;
    snapshotEntity.maxHp = remoteIter.value().maxHp;
    snapshotEntity.nickname = remoteIter.value().nickname;
    snapshotEntity.uiColor = uiColorFromColorId(remoteIter.value().colorId);
    snapshotEntity.isLocalPlayer = false;

    if (remoteIter.value().activeItemKind == 1) {
        snapshotEntity.slotIndicatorPixmap = buildResourceSlotIndicatorPixmap(remoteIter.value().activeResourceType);
    } else {
        snapshotEntity.slotIndicatorPixmap = buildKatanaSlotIndicatorPixmap();
    }
    snapshotEntity.showSlotIndicator = !snapshotEntity.slotIndicatorPixmap.isNull();
    ensureEntityUi(snapshotEntity);
}

void GameView::applyLocalPlayerKnockback(quint32 attackerPlayerId, int appliedDamage) {
    if (attackerPlayerId == 0) return;
    if (appliedDamage <= 0) return;

    Player* player = m_fakeServer.game().getPlayer();
    if (!player || !player->isAlive()) return;

    const auto remoteIter = m_remotePlayers.constFind(attackerPlayerId);
    if (remoteIter == m_remotePlayers.constEnd() || !remoteIter.value().initialized)
        return;

    const QPointF attackerPos = remoteIter.value().pos;

    QPointF hitDir = player->getPosition() - attackerPos;
    const double hitLen = std::hypot(hitDir.x(), hitDir.y());
    if (hitLen > 0.0)
        hitDir /= hitLen;

    const double knockbackStrength = std::clamp(appliedDamage * 2.0, 0.0, 500.0);
    player->addImpulse(hitDir * knockbackStrength);
}

void GameView::onLocalPlayerAttackPerformed(float dirX, float dirY) {
    emit localPlayerAttackProduced(m_localNetTick, dirX, dirY);
}

void GameView::onRemotePickupCollected(qint16 tileX, qint16 tileY) {
    const QPoint tilePos(tileX, tileY);
    if (!m_fakeServer.game().getMap().isInsideMap(tilePos))
        return;

    if (m_fakeServer.game().removePickupAtTile(tilePos))
        updatePickupsUi();
}

void GameView::onExternalPlayerHit(quint32 targetPlayerId, quint16 damage) {
    if (targetPlayerId == 0 || damage == 0) return;

    ensureRemotePlayerUi(targetPlayerId);
    startDamageFlashForEntity(remoteEntityIdFromPlayerId(targetPlayerId), int(damage));
    emit localPlayerHitProduced(targetPlayerId, damage);
}

void GameView::applyLocalPlayerHit(quint32 attackerPlayerId, quint32 targetPlayerId, quint16 damage) {
    Q_UNUSED(targetPlayerId);
    if (damage == 0) return;

    Player* player = m_fakeServer.game().getPlayer();
    if (!player) return;
    if (!player->isAlive()) return;

    const unsigned short hpBefore = player->getCurrentHp();
    player->takeDamage(int(damage));
    const unsigned short hpAfter = player->getCurrentHp();
    const int appliedDamage = std::max(0, int(hpBefore) - int(hpAfter));

    if (appliedDamage > 0) {
        ensureLocalPlayerUi();
        startDamageFlashForEntity(m_fakeServer.entityIdFor(player), appliedDamage);
        applyLocalPlayerKnockback(attackerPlayerId, appliedDamage);
    }
}

void GameView::onRemotePlayerAttack(quint32 attackerPlayerId, quint32 tick, float dirX, float dirY) {
    Q_UNUSED(tick);
    if (attackerPlayerId == 0) return;

    ensureRemotePlayerUi(attackerPlayerId);

    const auto remoteIter = m_remotePlayers.constFind(attackerPlayerId);
    if (remoteIter == m_remotePlayers.constEnd() || !remoteIter.value().initialized)
        return;

    const quint32 entityId = remoteEntityIdFromPlayerId(attackerPlayerId);
    if (entityId == 0 || !m_entityItems.contains(entityId))
        return;

    EntityUi& entityUi = m_entityItems[entityId];

    const QPointF playerPos = remoteIter.value().pos;
    const QPointF directionVector(dirX, dirY);
    {
        const QPainterPath attackPath = AttackFxUtils::buildKatanaIndicatorAttackPathWorld(playerPos, directionVector, getRemotePlayerRadius());
        if (attackPath.isEmpty()) {
            entityUi.attackFxParamsValid = false;
        } else {
            const QPainterPath clipped = AttackFxUtils::clipAttackPathToObstacles(attackPath, playerPos, m_fakeServer.game().getMap());
            const AttackFxUtils::Params attackFxParams = AttackFxUtils::computeAttackFxParamsFromShape(playerPos, directionVector, clipped.isEmpty() ? attackPath : clipped);
            entityUi.attackFxParamsValid = attackFxParams.valid;
            if (attackFxParams.valid) {
                entityUi.attackFxAngleDeg = attackFxParams.angleDeg;
                entityUi.attackFxSize = attackFxParams.size;
                entityUi.attackFxCenterOffset = attackFxParams.centerOffset;
            }
        }
    }

    triggerAttackAnimForEntity(entityId);
}

void GameView::startDamageFlashForEntity(quint32 entityId, int damage) {
    if (entityId == 0) return;
    if (damage <= 0) return;
    if (!m_entityItems.contains(entityId)) return;

    EntityUi& entityUi = m_entityItems[entityId];
    if (!entityUi.damageEffect) return;

    const float baseDuration = 0.06f;
    const float extraPerDamage = 0.002f;
    const float maxDuration = 0.25f;
    const float duration = std::clamp(baseDuration + damage * extraPerDamage, baseDuration, maxDuration);

    entityUi.damageFlashRemaining = duration;
    entityUi.damageFlashTotal = duration;
    entityUi.damageEffect->setStrength(1.0);
}

void GameView::setLocalPlayerNickname(const QString& nickname) {
    if (Player* player = m_fakeServer.game().getPlayer())
        player->setNickname(nickname);
}

void GameView::setLocalPlayerUiColor(const QColor& color) {
    if (Player* player = m_fakeServer.game().getPlayer()) {
        player->setUiColor(color);

        const quint32 entityId = m_fakeServer.entityIdFor(player);
        if (entityId != 0 && m_entityItems.contains(entityId) && m_entityItems[entityId].body) {
            QColor brushColor = color;
            if (brushColor.isValid()) brushColor.setAlpha(150);
            else brushColor = QColor(0, 200, 255, 150);

            m_entityItems[entityId].body->setBrush(brushColor);
        }
    }
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
    m_remotePlayers.clear();
    m_localNetTick = 0;

    m_useNetTimer = true;
    m_haveNetTimerSync = false;
    m_netTimerPhase = 0;
    m_netTimerMsLeftAtSync = 0;
    m_netTimerLastDrawPhase = 255;
    m_netTimerLastDrawSeconds = -1;
    if (!m_netTimerLocalClock.isValid()) m_netTimerLocalClock.start();

    m_darkAmount = 1.0f;
    m_darkOverlay->setOpacity(m_darkAmount);
    m_darkMode = DarkOverlayMode::FadeOut;

    m_gamePaused = true;
    m_timerLabel->setText("...");
    m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255, 200, 50);");
    m_timerLabel->setVisible(true);
}

void GameView::onGameTimeSync(quint8 phase, quint32 msLeft) {
    if (!m_useNetTimer) return;

    if (!m_netTimerLocalClock.isValid())
        m_netTimerLocalClock.start();

    const qint64 nowLocalMs = m_netTimerLocalClock.elapsed();
    const bool phaseChanged = (m_haveNetTimerSync && phase != m_netTimerPhase);

    m_haveNetTimerSync = true;
    m_netTimerPhase = phase;
    m_netTimerMsLeftAtSync = msLeft;
    m_netTimerLocalMsAtSync = nowLocalMs;

    if (phaseChanged) {
        m_netTimerLastDrawPhase = 255;
        m_netTimerLastDrawSeconds = -1;
    }

    // Phase transitions: server is authoritative.
    if (m_netTimerPhase == 1) {
        // Countdown
        m_gamePaused = true;
    } else if (m_netTimerPhase == 2) {
        // Game
        m_gamePaused = false;
        m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255, 255, 255);");
    } else {
        // Idle/ended
        if (!m_gamePaused) {
            m_gamePaused = true;
            m_darkMode = DarkOverlayMode::FadeIn;
            m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255, 20, 0);");
        }
    }
}

void GameView::onRemotePlayerState(const RemotePlayerStateUpdate& update) {
    if (update.playerId == 0) return;

    RemotePlayerNetState& state = m_remotePlayers[update.playerId];
    if (state.initialized && update.tick <= state.lastTick)
        return;

    const QPointF newPos(update.posX, update.posY);
    if (!state.initialized) {
        state.pos = newPos;
        state.prevPos = newPos;
        state.initialized = true;
    } else {
        state.prevPos = state.pos;
        state.pos = newPos;
    }

    state.hp = update.hp;
    state.maxHp = update.maxHp;
    state.nickname = update.nickname;
    state.colorId = update.colorId;
    state.activeItemKind = update.activeItemKind;
    state.activeResourceType = update.activeResourceType;
    state.aimDir = QPointF(update.aimDirX, update.aimDirY);
    state.lastTick = update.tick;
}

void GameView::setConnectedRemotePlayers(const QVector<quint32>& playerIds) {
    QSet<quint32> keep;
    keep.reserve(playerIds.size());
    for (quint32 playerId : playerIds)
        if (playerId != 0) keep.insert(playerId);

    for (auto iter = m_remotePlayers.begin(); iter != m_remotePlayers.end(); ) {
        if (!keep.contains(iter.key()))
            iter = m_remotePlayers.erase(iter);
        else
            ++iter;
    }
}

void GameView::appendRemotePlayersToSnapshot() {
    if (m_remotePlayers.isEmpty()) return;

    float remotePlayerRadius = 15.0f;
    if (const Player* player = m_fakeServer.game().getPlayer())
        remotePlayerRadius = player->getRadius();

    for (auto iter = m_remotePlayers.constBegin(); iter != m_remotePlayers.constEnd(); ++iter) {
        const quint32 playerId = iter.key();
        const RemotePlayerNetState& state = iter.value();
        if (!state.initialized) continue;
        if (state.hp == 0) continue;

        EntitySnapshot snapshotEntity;
        snapshotEntity.id = remoteEntityIdFromPlayerId(playerId);
        snapshotEntity.kind = SnapshotEntityKind::Player;
        snapshotEntity.alive = true;
        snapshotEntity.pos = state.pos;
        snapshotEntity.prevPos = state.prevPos;
        snapshotEntity.aimDir = state.aimDir;
        snapshotEntity.radius = remotePlayerRadius;
        snapshotEntity.hp = state.hp;
        snapshotEntity.maxHp = state.maxHp;
        snapshotEntity.nickname = state.nickname;
        snapshotEntity.uiColor = uiColorFromColorId(state.colorId);
        snapshotEntity.isLocalPlayer = false;

        snapshotEntity.showAttackIndicator = false;
        snapshotEntity.canAttack = false;

        if (state.activeItemKind == 1) {
            snapshotEntity.slotIndicatorPixmap = buildResourceSlotIndicatorPixmap(state.activeResourceType);
        } else {
            snapshotEntity.slotIndicatorPixmap = buildKatanaSlotIndicatorPixmap();
        }
        snapshotEntity.showSlotIndicator = !snapshotEntity.slotIndicatorPixmap.isNull();

        m_lastSnapshot.entities.push_back(snapshotEntity);
    }
}

void GameView::onCountdownTick(int secondsRemaining) {
    secondsRemaining = std::max(0, secondsRemaining);
    m_timerLabel->setText(QString::number(secondsRemaining));
    m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255, 200, 50);");
    m_timerLabel->setVisible(true);
}

void GameView::onGameStarted() {
    m_gamePaused = false;

    m_timerLabel->setStyleSheet(m_timerBaseStyle + "color: rgb(255, 255, 255);");
    m_gameTimer->startGameTimer(180);
}

void GameView::onGameTimerTick(int secondsRemaining) {
    secondsRemaining = std::max(0, secondsRemaining);
    int minute = secondsRemaining / 60;
    int second = secondsRemaining % 60;

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

    if (m_minimapWidget) {
        const int margin = 14;
        m_minimapWidget->move(viewport()->width() - m_minimapWidget->width() - margin, margin);
    }
}

void GameView::initSlotWidget() {
    m_slotWidget = new PlayerSlotWidget(this);
    m_slotWidget->setFixedSize(300, 70);

    m_slotWidget->raise();
    m_slotWidget->show();

    if (Player* player = m_fakeServer.game().getPlayer())
        m_slotWidget->setPlayer(player);
}

void GameView::initMiniMapWidget() {
    m_minimapWidget = new MiniMapWidget(this);
    m_minimapWidget->raise();
    m_minimapWidget->show();
    m_minimapWidget->setMap(&m_fakeServer.game().getMap());

    const int margin = 14;
    m_minimapWidget->move(viewport()->width() - m_minimapWidget->width() - margin, margin);
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

void GameView::initHpBar(EntityUi& entityUi) {
    const double barWidth = entityUi.body->boundingRect().width() * m_scaleSize;
    const double barHeight = 10;
    const QPointF barOffset(0, -barHeight - 6);

    entityUi.hpBack = createRectItem(entityUi.body, QRectF(0, 0, barWidth, barHeight), QColor(50,50,50), 10);
    entityUi.hpBack->setPos(barOffset);

    entityUi.hpFill = createRectItem(entityUi.hpBack, QRectF(0, 0, barWidth, barHeight), QColor(60,220,80), 11);
    entityUi.hpFill->setPos(0, 0);

    entityUi.hpTextMask = createRectItem(entityUi.hpBack, QRectF(0, 0, barWidth, barHeight), Qt::transparent, 13);
    entityUi.hpTextMask->setPos(0, 0);
    entityUi.hpTextMask->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    entityUi.hpTextWhite = createTextItem(entityUi.hpBack, Qt::white, 12);
    entityUi.hpTextBlack = createTextItem(entityUi.hpTextMask, Qt::black, 14);
}

void GameView::initNameTag(EntityUi& entityUi) {
    entityUi.nameTextWhite = createTextItem(entityUi.body, QColor(224, 224, 224, 200), 16, 9);
    entityUi.nameTextBlack = createTextItem(entityUi.body, QColor(0, 0, 0, 180), 15, 9);

    entityUi.nameTextWhite->hide();
    entityUi.nameTextBlack->hide();
}

void GameView::initAttackIndicator(EntityUi& entityUi) {
    entityUi.attackIndicator = new QGraphicsPathItem(entityUi.body);
    entityUi.attackIndicator->setBrush(QColor(150, 150, 150, 120));
    entityUi.attackIndicator->setPen(Qt::NoPen);
    entityUi.attackIndicator->hide();
}

void GameView::initSlotIndicator(EntityUi& entityUi) {
    entityUi.slotIndicator = new QGraphicsPixmapItem(entityUi.body);
    entityUi.slotIndicator->setZValue(6);
    entityUi.slotIndicator->hide();
}

void GameView::initDamageEffect(EntityUi& entityUi) {
    entityUi.damageEffect = new QGraphicsColorizeEffect();
    entityUi.damageEffect->setColor(QColor(255, 0, 0));
    entityUi.damageEffect->setStrength(0.0);
    entityUi.sprite->setGraphicsEffect(entityUi.damageEffect);
}

static QVector<QPixmap> scaleFrames(const QVector<QPixmap>& frames, const QSize& size) {
    QVector<QPixmap> scaledFrames;
    scaledFrames.reserve(frames.size());
    for (const QPixmap& framePixmap : frames) {
        if (framePixmap.isNull())
            continue;
        scaledFrames.push_back(framePixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    return scaledFrames;
}

static QVector<QPixmap> loadNumberedFrames(const QString& prefix, int firstIndex, int maxFrames) {
    QVector<QPixmap> loadedFrames;
    loadedFrames.reserve(std::max(0, maxFrames));

    for (int frameOffset = 0; frameOffset < maxFrames; ++frameOffset) {
        const int frameIndex = firstIndex + frameOffset;
        QPixmap pixmap(prefix + QString::number(frameIndex) + ".png");
        if (pixmap.isNull())
            break;
        loadedFrames.push_back(pixmap);
    }

    return loadedFrames;
}

static QVector<QPixmap> loadKatanaAttackFrames() {
    QVector<QPixmap> loadedFrames;
    loadedFrames.reserve(8);

    for (int frameIndex = 1; frameIndex <= 32; ++frameIndex) {
        QPixmap pixmap(QString(":/attack/katana/") + QString::number(frameIndex) + ".png");
        if (pixmap.isNull())
            break;
        loadedFrames.push_back(pixmap);
    }

    return loadedFrames;
}

GameView::EntityUi& GameView::ensureEntityUi(const EntitySnapshot& snapshotEntity) {
    if (m_entityItems.contains(snapshotEntity.id))
        return m_entityItems[snapshotEntity.id];

    EntityUi entityUi;

    initEntityBodyVisual(snapshotEntity, entityUi);
    initEntitySpritesAndAnimations(snapshotEntity, entityUi);

    initHpBar(entityUi);
    initNameTag(entityUi);
    initAttackIndicator(entityUi);
    initSlotIndicator(entityUi);
    initDamageEffect(entityUi);

    m_entityItems.insert(snapshotEntity.id, entityUi);
    return m_entityItems[snapshotEntity.id];
}

void GameView::initEntityBodyVisual(const EntitySnapshot& snapshotEntity, EntityUi& entityUi) {
    const float radius = snapshotEntity.radius;

    entityUi.body = new QGraphicsEllipseItem(0, 0, radius * 2.0f, radius * 2.0f);
    if (snapshotEntity.kind == SnapshotEntityKind::Player) {
        QColor brushColor = snapshotEntity.uiColor;
        if (brushColor.isValid())
            brushColor.setAlpha(150);
        else
            brushColor = QColor(0, 200, 255, 150);
        entityUi.body->setBrush(brushColor);
    } else {
        entityUi.body->setBrush(QColor(255, 50, 50, 150));
    }

    entityUi.body->setPen(Qt::NoPen);
    entityUi.body->setZValue(5);
    m_scene->addItem(entityUi.body);
    entityUi.body->setPos(snapshotEntity.pos - QPointF(radius, radius));
}

void GameView::initEntitySpritesAndAnimations(const EntitySnapshot& snapshotEntity, EntityUi& entityUi) {
    const float radius = snapshotEntity.radius;
    const QSize spriteSize(int(radius * 2.0f), int(radius * 2.0f));

    QPixmap standPixmap(":/entities/tank/tank_stand.png");
    if (standPixmap.isNull())
        standPixmap = QPixmap(":/entities/tank.png");

    entityUi.baseSprite = standPixmap.scaled(spriteSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (entityUi.baseSprite.isNull())
        entityUi.baseSprite = QPixmap(spriteSize);

    QVector<QPixmap> walkFrames = scaleFrames(loadNumberedFrames(":/entities/tank/tank_walk_", 1, 32), spriteSize);
    if (walkFrames.isEmpty())
        walkFrames = { entityUi.baseSprite };

    QVector<QPixmap> attackFrames = scaleFrames(loadKatanaAttackFrames(), spriteSize);
    if (attackFrames.isEmpty())
        attackFrames = { entityUi.baseSprite };

    entityUi.walkAnim = SpriteAnim(walkFrames, 12.0f, true);
    entityUi.walkAnim.stop();
    entityUi.attackAnim = SpriteAnim(attackFrames, 18.0f, false);
    entityUi.attackAnim.stop();

    entityUi.sprite = new QGraphicsPixmapItem(entityUi.baseSprite, entityUi.body);
    entityUi.sprite->setZValue(5);
    entityUi.sprite->setTransformOriginPoint(entityUi.sprite->boundingRect().center());

    entityUi.attackFx = new QGraphicsPixmapItem(entityUi.body);
    entityUi.attackFx->setZValue(7);
    entityUi.attackFx->setTransformOriginPoint(0, 0);
    entityUi.attackFx->setOpacity(0.9);
    entityUi.attackFx->hide();
}

static double len2(const QPointF& movementVector) {
    return movementVector.x() * movementVector.x() + movementVector.y() * movementVector.y();
}

void GameView::updateEntityRotationFromMovement(const QPointF& movementVector, EntityUi& entityUi) {
    if (movementVector.isNull() || !entityUi.sprite)
        return;

    const double rotationDegrees = qRadiansToDegrees(std::atan2(movementVector.y(), movementVector.x())) - 90.0;
    entityUi.sprite->setRotation(rotationDegrees);
}

void GameView::updateEntityWalkAnimationFromMovement(const QPointF& movementVector, EntityUi& entityUi, bool allowAnimations) {
    const bool isMoving = allowAnimations && (len2(movementVector) > 0.01);

    if (isMoving) {
        if (!entityUi.walkActive) {
            entityUi.walkActive = true;
            entityUi.walkAnim.reset();
            entityUi.walkAnim.play();
        }
    } else {
        if (entityUi.walkActive) {
            entityUi.walkActive = false;
            entityUi.walkAnim.reset();
            entityUi.walkAnim.stop();
        }
    }
}

void GameView::updateEntityAttackAnimation(EntityUi& entityUi, bool allowAnimations) {
    if (!allowAnimations) {
        entityUi.attackActive = false;
        entityUi.attackAnim.stop();
        if (entityUi.attackFx)
            entityUi.attackFx->hide();
        return;
    }

    if (entityUi.attackActive && entityUi.attackAnim.finished()) {
        entityUi.attackActive = false;
        entityUi.attackAnim.stop();
        if (entityUi.attackFx)
            entityUi.attackFx->hide();
    }
}

void GameView::updateEntityBodySprite(EntityUi& entityUi) {
    const QPixmap* bodyFrame = &entityUi.baseSprite;
    if (entityUi.walkActive)
        bodyFrame = &entityUi.walkAnim.tick(m_deltaTime);

    if (entityUi.sprite && bodyFrame && !bodyFrame->isNull())
        entityUi.sprite->setPixmap(*bodyFrame);

    if (entityUi.sprite) {
        if (entityUi.walkActive && entityUi.walkAnim.getFrames().size() <= 1) {
            const double bobOffset = std::sin(m_animTime * 12.0) * 1.5;
            entityUi.sprite->setOffset(0.0, bobOffset);
        } else {
            entityUi.sprite->setOffset(0.0, 0.0);
        }

        entityUi.sprite->setScale(1.0);
    }
}

void GameView::updateEntityAttackFx(const EntitySnapshot& entity, EntityUi& entityUi) {
    if (!entityUi.attackFx)
        return;

    if (!entityUi.attackActive) {
        entityUi.attackFx->hide();
        return;
    }

    const QPixmap& fxFrame = entityUi.attackAnim.tick(m_deltaTime);
    if (fxFrame.isNull())
        return;

    entityUi.attackFx->setPixmap(fxFrame);

    const double offsetX = fxFrame.width() / 2.0;
    const double offsetY = fxFrame.height() / 2.0;
    entityUi.attackFx->setOffset(-offsetX, -offsetY);
    entityUi.attackFx->setTransformOriginPoint(0, 0);

    const QPointF bodyCenter(entity.radius, entity.radius);

    if (entityUi.attackFxParamsValid) {
        const double desiredWidth = std::max(1.0, entityUi.attackFxSize.width());
        const double desiredHeight = std::max(1.0, entityUi.attackFxSize.height());

        const double scaleX = desiredWidth / std::max(1.0, double(fxFrame.width()));
        const double scaleY = desiredHeight / std::max(1.0, double(fxFrame.height()));

        const double angleRadians = entityUi.attackFxAngleDeg * M_PI / 180.0;
        const double cosAngle = std::cos(angleRadians);
        const double sinAngle = std::sin(angleRadians);

        const QTransform transform(scaleX * cosAngle, scaleX * sinAngle, -scaleY * sinAngle, scaleY * cosAngle, 0.0, 0.0);
        entityUi.attackFx->setTransform(transform);
        entityUi.attackFx->setPos(bodyCenter + entityUi.attackFxCenterOffset);
    } else {
        const QPointF aimDirection = !entity.aimDir.isNull() ? entity.aimDir : (entity.pos - entity.prevPos);
        const double angleRadians = aimDirection.isNull() ? 0.0 : std::atan2(aimDirection.y(), aimDirection.x());
        const double angleDegrees = qRadiansToDegrees(angleRadians);

        const double rangeDistance = 60.0;
        const QPointF localOffset(std::cos(angleRadians) * (rangeDistance * 0.65),
                                  std::sin(angleRadians) * (rangeDistance * 0.65));

        QTransform transform;
        transform.scale(1.0, 1.0);
        transform.rotate(angleDegrees);
        entityUi.attackFx->setTransform(transform);
        entityUi.attackFx->setPos(bodyCenter + localOffset);
    }

    entityUi.attackFx->show();
}

void GameView::triggerAttackAnimForEntity(quint32 entityId) {
    if (entityId == 0) return;
    if (!m_entityItems.contains(entityId)) return;

    EntityUi& entityUi = m_entityItems[entityId];
    entityUi.attackActive = true;
    entityUi.attackAnim.reset();
    entityUi.attackAnim.play();
    if (entityUi.attackFx)
        entityUi.attackFx->show();
}

void GameView::initEntitiesUi() {
    QPointF mouseScene = mapToScene(mapFromGlobal(QCursor::pos()));
    m_lastSnapshot = m_fakeServer.makeSnapshot(mouseScene);

    QSet<quint32> seenIds;
    for (const EntitySnapshot& snapshotEntity : m_lastSnapshot.entities) {
        ensureEntityUi(snapshotEntity);
        seenIds.insert(snapshotEntity.id);
    }
    removeMissingEntityUis(seenIds);
}

void GameView::removeMissingEntityUis(const QSet<quint32>& seenIds) {
    for (auto iter = m_entityItems.begin(); iter != m_entityItems.end(); ) {
        if (seenIds.contains(iter.key())) {
            ++iter;
            continue;
        }

        EntityUi& entityUi = iter.value();
        if (entityUi.body) {
            m_scene->removeItem(entityUi.body);
            delete entityUi.body;
        }
        iter = m_entityItems.erase(iter);
    }
}

void GameView::onEnemyHit(const HitInfo& hit) {
    Entity* target = hit.target;
    const quint32 targetId = m_fakeServer.entityIdFor(target);
    if (!target || targetId == 0 || !m_entityItems.contains(targetId)) return;

    if (hit.damage <= 0) return;

    startDamageFlashForEntity(targetId, hit.damage);
}

void GameView::initBuildPreview() {
    m_buildPreview = new QGraphicsPixmapItem();
    m_buildPreview->setOpacity(0.5);
    m_buildPreview->setZValue(3);
    m_buildPreview->hide();
    m_scene->addItem(m_buildPreview);
}

void GameView::buildMap() {
    const Map& gameMap = m_fakeServer.game().getMap();

    int mapWidth = gameMap.getTileCountX() * Map::TILE_SIZE;
    int mapHeight = gameMap.getTileCountY() * Map::TILE_SIZE;
    double offsetX = -mapWidth / 2.0;
    double offsetY = -mapHeight / 2.0;

    initBuildPreview();

    for (int tileY = 0; tileY < gameMap.getTileCountY(); ++tileY) {
        for (int tileX = 0; tileX < gameMap.getTileCountX(); ++tileX) {

            QGraphicsPixmapItem* tileItem = new QGraphicsPixmapItem();
            tileItem->setPos(tileX * Map::TILE_SIZE + offsetX, tileY * Map::TILE_SIZE + offsetY);
            tileItem->setZValue(0);

            const Tile& tile = gameMap.tileAt(tileX, tileY);
            const TileVisual& visual = tile.getType() == Tile::TileType::Empty ? emptyVisual(tile.getVariation())
                                                                               : tileVisual(tile.getType());
            if (!visual.sprite.isNull())
                tileItem->setPixmap(visual.sprite.scaled(Map::TILE_SIZE, Map::TILE_SIZE,
                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

            m_scene->addItem(tileItem);
            m_tileItems[QPoint(tileX, tileY)] = tileItem;
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
        m_fakeServer.setPlayerInput(m_moveKeyMap[scanCode], pressed);

    Qt::Key pressedKey = static_cast<Qt::Key>(event->key());
    if (pressed && m_slotKeyMap.contains(pressedKey))
        m_fakeServer.setActiveSlot(m_slotKeyMap[pressedKey]);
    // qDebug() << "key:" << event->key()
    //         << "scan:" << event->nativeScanCode();
}

void GameView::keyPressEvent(QKeyEvent* event) { handleKeyEvent(event, true); }
void GameView::keyReleaseEvent(QKeyEvent* event) { handleKeyEvent(event, false); }

void GameView::focusOutEvent(QFocusEvent* event) {
    QGraphicsView::focusOutEvent(event);

    m_fakeServer.setPlayerInput(MoveDirection::MoveUp, false);
    m_fakeServer.setPlayerInput(MoveDirection::MoveDown, false);
    m_fakeServer.setPlayerInput(MoveDirection::MoveLeft, false);
    m_fakeServer.setPlayerInput(MoveDirection::MoveRight, false);
}

void GameView::mousePressEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) m_fakeServer.startAiming();
}

void GameView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;

    Player* player = m_fakeServer.game().getPlayer();
    if (!player) return;

    const QPointF mouseScene = mapToScene(event->pos());

    if (player->getInventory().isActiveResource())
        m_fakeServer.tryPlaceTile(m_fakeServer.game().getMap().worldToTile(mouseScene));
    else {
        const bool willAttack = player->canAttack() && player->getInventory().getActiveWeapon();

        if (willAttack) {
            ensureLocalPlayerUi();
            const quint32 localEntityId = m_fakeServer.entityIdFor(player);

            if (localEntityId != 0 && m_entityItems.contains(localEntityId)) {
                EntityUi& entityUi = m_entityItems[localEntityId];

                const QPointF playerPos = player->getPosition();
                const QPointF directionVector = mouseScene - playerPos;

                const QPainterPath shapeWorld = m_fakeServer.game().getPlayerAttackShape(mouseScene);
                if (!shapeWorld.isEmpty()) {
                    const AttackFxUtils::Params attackFxParams = AttackFxUtils::computeAttackFxParamsFromShape(playerPos, directionVector, shapeWorld);
                    entityUi.attackFxParamsValid = attackFxParams.valid;
                    if (attackFxParams.valid) {
                        entityUi.attackFxAngleDeg = attackFxParams.angleDeg;
                        entityUi.attackFxSize = attackFxParams.size;
                        entityUi.attackFxCenterOffset = attackFxParams.centerOffset;
                    }
                } else {
                    entityUi.attackFxParamsValid = false;
                }
            }
        }

        m_fakeServer.stopAiming(mouseScene - player->getPosition());

        if (willAttack) {
            const quint32 localEntityId = m_fakeServer.entityIdFor(player);
            triggerAttackAnimForEntity(localEntityId);
        }
    }
}

void GameView::wheelEvent(QWheelEvent* event) { event->ignore(); }

void GameView::setupSlotKeys(const QVector<Qt::Key>& keys) {
    m_slotKeyMap.clear();

    for (int slot = 0; slot < keys.size(); ++slot) {
        Qt::Key slotKey = keys[slot];
        if (slotKey != Qt::Key_unknown)
            m_slotKeyMap[slotKey] = slot;
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
        scanUp = 57416; // Up
        scanDown = 57424; // Down
        scanLeft = 57419; // Left
        scanRight = 57421; // Right
    }

    m_moveKeyMap.clear();
    m_moveKeyMap[scanUp] = MoveDirection::MoveUp;
    m_moveKeyMap[scanDown] = MoveDirection::MoveDown;
    m_moveKeyMap[scanLeft] = MoveDirection::MoveLeft;
    m_moveKeyMap[scanRight] = MoveDirection::MoveRight;
}

void GameView::prepareSlotKeysVector() {
    QVector<Qt::Key> slotKeysVector;
    slotKeysVector.resize(m_slotKeyMap.size());

    for (QMap<Qt::Key, unsigned short>::iterator iter = m_slotKeyMap.begin(); iter != m_slotKeyMap.end(); ++iter) {
        Qt::Key slotKey = iter.key();
        int slotIndex = iter.value();
        if (slotIndex >= 0 && slotIndex < slotKeysVector.size())
            slotKeysVector[slotIndex] = slotKey;
    }

    if (m_slotWidget)
        m_slotWidget->setSlotKeys(slotKeysVector);
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
    for (const EntitySnapshot& entitySnapshot : m_lastSnapshot.entities) {
        if (!entitySnapshot.isLocalPlayer || !entitySnapshot.alive) continue;
        m_cameraPos = m_cameraPos * 0.95 + entitySnapshot.pos * 0.05;
        centerOn(m_cameraPos);
        return;
    }
}

void GameView::updateSlotWidget() {
    if (m_slotWidget && m_fakeServer.game().getPlayer())
        m_slotWidget->update();
}

QColor getHpBarColor(double ratio) {
    unsigned short redValue, greenValue;
    double progress;

    if (ratio > 0.75) {
        progress = (ratio - 0.75) / 0.25;
        redValue = int(60   + (255 - 60) * (1.0 - progress));
        greenValue = 255;
    } else if (ratio > 0.5) {
        progress = (ratio - 0.5) / 0.25;
        redValue = 255;
        greenValue = int(165 + (255 - 165) * progress);
    } else {
        progress = ratio / 0.5;
        redValue = 255;
        greenValue = int(165 * progress);
    }

    return QColor(redValue, greenValue, 0);
}

void GameView::updateEntityHp(const EntitySnapshot& entity, EntityUi& entityUi) {
    const double denom = (entity.maxHp > 0) ? double(entity.maxHp) : 1.0;
    const double ratio = double(entity.hp) / denom;
    const QRectF backRect = entityUi.hpBack->rect();
    entityUi.hpFill->setRect(0, 0, backRect.width() * ratio, backRect.height());
    entityUi.hpFill->setBrush(getHpBarColor(ratio));

    const QString text = QString::number(entity.hp);
    entityUi.hpTextWhite->setPlainText(text);
    entityUi.hpTextBlack->setPlainText(text);

    const QRectF textRect = entityUi.hpTextWhite->boundingRect();
    const double fillWidth = backRect.width() * ratio;

    entityUi.hpTextMask->setRect(0, 0, fillWidth, backRect.height());
    entityUi.hpTextWhite->setPos(backRect.width()/2 - textRect.width()/2,
                                 backRect.height()/2 - textRect.height()/2);
    entityUi.hpTextBlack->setPos(entityUi.hpTextWhite->pos());
}

void GameView::updateEntityName(const EntitySnapshot& entity, EntityUi& entityUi) {
    if (!entityUi.nameTextWhite || !entityUi.nameTextBlack || !entityUi.hpBack) {
        if (entityUi.nameTextWhite) entityUi.nameTextWhite->hide();
        if (entityUi.nameTextBlack) entityUi.nameTextBlack->hide();
        return;
    }

    if (entity.kind != SnapshotEntityKind::Player) {
        entityUi.nameTextWhite->hide();
        entityUi.nameTextBlack->hide();
        return;
    }

    const QString nickname = entity.nickname;
    if (nickname.trimmed().isEmpty()) {
        entityUi.nameTextWhite->hide();
        entityUi.nameTextBlack->hide();
        return;
    }

    const QRectF backRect = entityUi.hpBack->rect();

    entityUi.nameTextWhite->setPlainText(nickname);
    entityUi.nameTextBlack->setPlainText(nickname);

    const QRectF textRect = entityUi.nameTextWhite->boundingRect();
    const QPointF hpPos = entityUi.hpBack->pos();
    const double namePosX = hpPos.x() + backRect.width() / 2.0 - textRect.width() / 2.0;
    const double namePosY = hpPos.y() - textRect.height() + 3.0;

    entityUi.nameTextWhite->setPos(namePosX, namePosY);
    entityUi.nameTextBlack->setPos(namePosX + 1.0, namePosY + 1.0);

    entityUi.nameTextWhite->show();
    entityUi.nameTextBlack->show();
}

void GameView::updateEntityAtkIndicator(const EntitySnapshot& entity, EntityUi& entityUi) {
    if (!entityUi.attackIndicator) return;
    if (!entity.showAttackIndicator || entity.attackIndicatorLocalPath.isEmpty()) {
        entityUi.attackIndicator->hide();
        return;
    }

    entityUi.attackIndicator->setBrush(entity.canAttack ? QColor(255, 220, 100, 120)
                                                        : QColor(150, 150, 150, 120));
    entityUi.attackIndicator->setPath(entity.attackIndicatorLocalPath);
    QPointF center(entity.radius / 2.0f, entity.radius / 2.0f);
    entityUi.attackIndicator->setPos(center);
    entityUi.attackIndicator->show();
}

void GameView::updateEntitiesVisibility(const EntitySnapshot& entity, EntityUi& entityUi) {
    const float radius = entity.radius;
    QRectF entityRect(entity.pos - QPointF(radius, radius), QSizeF(radius * 2, radius * 2));

    bool inGrass = m_fakeServer.game().getMap().intersectsGrass(entityRect);

    if (entity.isLocalPlayer) {
        entityUi.body->setVisible(true);
        entityUi.body->setOpacity(inGrass ? 0.5 : 1.0);
    } else {
        entityUi.body->setOpacity(1.0);
        entityUi.body->setVisible(!inGrass);
    }
}

void GameView::updateEntitySlotIndicator(const EntitySnapshot& entity, EntityUi& entityUi, const QPointF& mouseScene) {
    if (!entityUi.slotIndicator) return;

    if (!entity.showSlotIndicator || entity.slotIndicatorPixmap.isNull()) {
        entityUi.slotIndicator->hide();
        return;
    }

    const QPixmap& pixmap = entity.slotIndicatorPixmap;
    entityUi.slotIndicator->setPixmap(pixmap);
    entityUi.slotIndicator->setOffset(-pixmap.width() / 2.0, -pixmap.height() / 2.0);
    entityUi.slotIndicator->setTransformOriginPoint(0, 0);
    entityUi.slotIndicator->show();

    QPointF playerCenterScene = entityUi.body->sceneBoundingRect().center();

    QPointF directionVector;
    if (entity.isLocalPlayer) {
        directionVector = mouseScene - playerCenterScene;
    } else if (!entity.aimDir.isNull()) {
        directionVector = entity.aimDir;
    } else {
        directionVector = entity.pos - entity.prevPos;
    }

    if (directionVector.isNull())
        directionVector = QPointF(1.0, 0.0);

    const double angle = std::atan2(directionVector.y(), directionVector.x());

    double orbitRadius = entity.radius + 2.0;

    QPointF localPos(std::cos(angle) * orbitRadius, std::sin(angle) * orbitRadius);
    QPointF bodyCenter(entity.radius, entity.radius);
    entityUi.slotIndicator->setPos(bodyCenter + localPos);

    entityUi.slotIndicator->setRotation(qRadiansToDegrees(angle));
}

void GameView::updateDamageFlash(EntityUi& entityUi) {
    if (!entityUi.damageEffect) return;

    if (entityUi.damageFlashRemaining <= 0.0f) {
        entityUi.damageEffect->setStrength(0.0);
        return;
    }

    entityUi.damageFlashRemaining = std::max(0.0f, entityUi.damageFlashRemaining - m_deltaTime);
    const float denom = (entityUi.damageFlashTotal > 0.0001f) ? entityUi.damageFlashTotal : 1.0f;
    const float strength = entityUi.damageFlashRemaining / denom;
    entityUi.damageEffect->setStrength(strength);
}

void GameView::updateSingleEntityUi(const EntitySnapshot& entity, QSet<quint32>& seenIds) {
    seenIds.insert(entity.id);
    EntityUi& entityUi = ensureEntityUi(entity);

    if (!entity.alive) {
        entityUi.body->hide();
        return;
    }

    entityUi.body->show();
    entityUi.body->setPos(entity.pos - QPointF(entity.radius, entity.radius));

    const QPointF movementVector = entity.pos - entity.prevPos;
    updateEntityRotationFromMovement(movementVector, entityUi);

    const bool allowAnimations = !m_gamePaused;

    updateEntityWalkAnimationFromMovement(movementVector, entityUi, allowAnimations);
    updateEntityAttackAnimation(entityUi, allowAnimations);
    updateEntityBodySprite(entityUi);
    updateEntityAttackFx(entity, entityUi);

    updateEntitiesVisibility(entity, entityUi);
    updateEntityHp(entity, entityUi);
    updateEntityName(entity, entityUi);
    updateEntityAtkIndicator(entity, entityUi);
    updateEntitySlotIndicator(entity, entityUi, m_lastMouseScene);
    updateDamageFlash(entityUi);
}

void GameView::updateEntitiesUi() {
    QSet<quint32> seenIds;
    for (const EntitySnapshot& entity : m_lastSnapshot.entities)
        updateSingleEntityUi(entity, seenIds);

    removeMissingEntityUis(seenIds);
}

void GameView::updateTile(int tileX, int tileY) {
    const QPoint tilePos(tileX, tileY);
    if (m_tileItems.contains(tilePos)) {
        const Tile& tile = m_fakeServer.game().getMap().tileAt(tileX, tileY);
        const TileVisual& visual = tile.getType() == Tile::TileType::Empty ? emptyVisual(tile.getVariation())
                                                                           : tileVisual(tile.getType());

        if (visual.sprite.isNull()) {
            m_tileItems[tilePos]->setPixmap(QPixmap());
            return;
        }

        m_tileItems[tilePos]->setPixmap(visual.sprite.scaled(Map::TILE_SIZE, Map::TILE_SIZE,
                                                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        if (m_minimapWidget)
            m_minimapWidget->notifyTileChanged(tileX, tileY);

        if (!m_applyingRemoteTile)
            emit localTileChanged(qint16(tileX), qint16(tileY), quint8(tile.getType()));
    }
}

void GameView::onRemoteTileUpdate(qint16 tileX, qint16 tileY, quint8 tileType) {
    const QPoint tilePos(tileX, tileY);
    if (!m_fakeServer.game().getMap().isInsideMap(tilePos))
        return;

    Map& gameMap = m_fakeServer.game().getMap();
    Tile& tile = gameMap.tileAt(tileX, tileY);
    const Tile::TileType oldType = tile.getType();
    const Tile::TileType newType = static_cast<Tile::TileType>(tileType);
    if (oldType == newType)
        return;

    m_applyingRemoteTile = true;
    tile.setType(newType);
    if (newType == Tile::TileType::Empty)
        m_fakeServer.game().maybeSpawnPickupForBrokenTile(tilePos, oldType);
    updateTile(tileX, tileY);
    m_applyingRemoteTile = false;
}

void GameView::updatePickupsUi() {
    for (const PickupItem* pickup : m_fakeServer.game().getPickups())
        if (!m_pickupItems.contains(pickup)) createPickupUi(pickup);

    for (auto iter = m_pickupItems.begin(); iter != m_pickupItems.end(); ) {
        if (!m_fakeServer.game().getPickups().contains(iter.key())) {
            delete iter.value();
            iter = m_pickupItems.erase(iter);
        } else ++iter;
    }
}

void GameView::updateBuildPreview() {
    Player* player = m_fakeServer.game().getPlayer();
    if (!player || !player->getInventory().isActiveResource()) {
        m_buildPreview->hide();
        return;
    }

    QPointF mouseScene = mapToScene(mapFromGlobal(QCursor::pos()));
    QPoint tile = m_fakeServer.game().getMap().worldToTile(mouseScene);
    Tile::TileType type = player->getInventory().getActiveResourceType();

    if (tile == m_previewTile && type == m_previewType) return;

    m_previewTile = tile;
    m_previewType = type;

    if (!m_fakeServer.game().canPlaceTile(tile)) {
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

    QPointF world = m_fakeServer.game().getMap().tileToWorld(tile);
    m_buildPreview->setPos(world - QPointF(Map::TILE_SIZE / 2, Map::TILE_SIZE / 2));

    m_buildPreview->show();
}

void GameView::updateSimulationTick() {
    m_gameTimer->update(m_deltaTime);
    if (m_gamePaused) return;

    QVector<QPointF> centers;
    QVector<float> radii;
    QVector<quint32> ids;

    centers.reserve(m_remotePlayers.size());
    radii.reserve(m_remotePlayers.size());
    ids.reserve(m_remotePlayers.size());

    const float remoteRadius = getRemotePlayerRadius();
    for (auto remoteIter = m_remotePlayers.constBegin(); remoteIter != m_remotePlayers.constEnd(); ++remoteIter) {
        const quint32 playerId = remoteIter.key();
        const RemotePlayerNetState& remoteState = remoteIter.value();
        if (!remoteState.initialized) continue;
        if (remoteState.hp == 0) continue;

        centers.push_back(remoteState.pos);
        radii.push_back(remoteRadius);
        ids.push_back(playerId);
    }

    m_fakeServer.setExternalCollisionCircles(centers, radii);
    m_fakeServer.setExternalHitPlayers(ids, centers, radii);
    m_fakeServer.update(m_deltaTime);
}

void GameView::updateSnapshotTick() {
    m_lastMouseScene = mapToScene(mapFromGlobal(QCursor::pos()));
    m_lastSnapshot = m_fakeServer.makeSnapshot(m_lastMouseScene);
    appendRemotePlayersToSnapshot();
}

void GameView::publishLocalPlayerStateTick() {
    if (m_gamePaused) return;

    ++m_localNetTick;
    LocalPlayerStateUpdate update;
    update.tick = m_localNetTick;

    if (const Player* player = m_fakeServer.game().getPlayer()) {
        if (player->getInventory().isActiveResource()) {
            update.activeItemKind = 1;
            update.activeResourceType = quint8(player->getInventory().getActiveResourceType());
        }

        QPointF aimDir = m_lastMouseScene - player->getPosition();
        const double aimLen = std::hypot(aimDir.x(), aimDir.y());
        if (aimLen > 0.0001) {
            aimDir /= aimLen;
            update.aimDirX = float(aimDir.x());
            update.aimDirY = float(aimDir.y());
        }
    }

    for (const EntitySnapshot& entity : m_lastSnapshot.entities) {
        if (!entity.isLocalPlayer) continue;
        update.posX = float(entity.pos.x());
        update.posY = float(entity.pos.y());
        update.hp = entity.hp;
        update.maxHp = entity.maxHp;
        emit localPlayerStateProduced(update);
        break;
    }
}

void GameView::updateUiTick() {
    m_animTime += m_deltaTime;
    updateDarkOverlay();
    updateCamera();
    updateSlotWidget();
    updateEntitiesUi();
    updatePickupsUi();
    updateBuildPreview();

    if (m_useNetTimer && m_haveNetTimerSync) {
        const qint64 nowLocalMs = m_netTimerLocalClock.elapsed();
        const qint64 elapsedSinceSyncMs = nowLocalMs - m_netTimerLocalMsAtSync;
        qint64 msLeft = qint64(m_netTimerMsLeftAtSync) - elapsedSinceSyncMs;
        if (msLeft < 0) msLeft = 0;

        const int secondsLeft = int((msLeft + 999) / 1000);
        if (m_netTimerPhase != m_netTimerLastDrawPhase || secondsLeft != m_netTimerLastDrawSeconds) {
            m_netTimerLastDrawPhase = m_netTimerPhase;
            m_netTimerLastDrawSeconds = secondsLeft;

            if (m_netTimerPhase == 1) {
                onCountdownTick(secondsLeft);
            } else if (m_netTimerPhase == 2) {
                onGameTimerTick(secondsLeft);
            } else {
                // Idle: keep whatever style onGameTimeSync set.
            }
        }
    }

    if (m_minimapWidget) {
        m_minimapWidget->setSnapshot(m_lastSnapshot);
        m_minimapWidget->setCameraRect(mapToScene(viewport()->rect()).boundingRect());
    }
}

void GameView::onTick() {
    updateSimulationTick();
    updateSnapshotTick();
    publishLocalPlayerStateTick();
    updateUiTick();
}
