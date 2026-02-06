#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsColorizeEffect>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <QColor>
#include <QSet>
#include <QElapsedTimer>
#include <cstdlib>
#include <ctime>

#include "../input/inputtypes.h"
#include "../ui/playerslotwidget.h"
#include "../ui/minimapwidget.h"
#include "fakeserver.h"
#include "fakesnapshot.h"
#include "spriteanim.h"
#include "gametimer.h"
#include "hitinfo.h"
#include "attackfxutils.h"
#include "../net/playerstateupdates.h"

class GameView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GameView(QWidget* parent = nullptr);

    void useMovementScheme(MovementScheme);
    void setupSlotKeys(const QVector<Qt::Key>&);
    void prepareSlotKeysVector();

    void startGameWithCountdown();

    void setLocalPlayerNickname(const QString&);
    void setLocalPlayerUiColor(const QColor&);

signals:
    void localPlayerStateProduced(const LocalPlayerStateUpdate&);
    void localTileChanged(qint16 tileX, qint16 tileY, quint8 tileType);
    void localPlayerHitProduced(quint32 targetPlayerId, quint16 damage);
    void localPickupCollected(qint16 tileX, qint16 tileY);
    void localPlayerAttackProduced(quint32 tick, float dirX, float dirY);

public slots:
    void onRemotePlayerState(const RemotePlayerStateUpdate&);
    void onRemoteTileUpdate(qint16 tileX, qint16 tileY, quint8 tileType);
    void onRemotePickupCollected(qint16 tileX, qint16 tileY);
    void onRemotePlayerAttack(quint32 attackerPlayerId, quint32 tick, float dirX, float dirY);
    void onGameTimeSync(quint8 phase, quint32 msLeft);
    void setConnectedRemotePlayers(const QVector<quint32>& playerIds);
    void applyLocalPlayerHit(quint32 attackerPlayerId, quint32 targetPlayerId, quint16 damage);

protected:
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

    void focusOutEvent(QFocusEvent*) override;

    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

    void wheelEvent(QWheelEvent*) override;

    void resizeEvent(QResizeEvent*) override;


private:
    void onExternalPlayerHit(quint32 targetPlayerId, quint16 damage);
    void triggerAttackAnimForEntity(quint32 entityId);

    void onLocalPlayerAttackPerformed(float dirX, float dirY);

    struct EntityUi {
        QGraphicsEllipseItem* body = nullptr;
        QGraphicsPixmapItem* sprite = nullptr;
        QPixmap baseSprite;

        SpriteAnim walkAnim;
        SpriteAnim attackAnim;

        bool walkActive = false;
        bool attackActive = false;

        QGraphicsPixmapItem* attackFx = nullptr;
        bool attackFxParamsValid = false;
        QPointF attackFxCenterOffset;
        QSizeF attackFxSize;
        double attackFxAngleDeg = 0.0;

        QGraphicsColorizeEffect* damageEffect = nullptr;
        float damageFlashRemaining = 0.0f;
        float damageFlashTotal = 0.0f;
        QGraphicsRectItem* hpBack = nullptr;
        QGraphicsRectItem* hpFill = nullptr;
        QGraphicsRectItem* hpTextMask = nullptr;
        QGraphicsTextItem* hpTextWhite = nullptr;
        QGraphicsTextItem* hpTextBlack = nullptr;
        QGraphicsTextItem* nameTextWhite = nullptr;
        QGraphicsTextItem* nameTextBlack = nullptr;
        QGraphicsPathItem* attackIndicator = nullptr;
        QGraphicsPixmapItem* slotIndicator = nullptr;
    };

    FakeServer m_fakeServer;
    WorldSnapshot m_lastSnapshot;
    QPointF m_lastMouseScene;
    quint32 m_localNetTick = 0;

    struct RemotePlayerNetState {
        QPointF pos;
        QPointF prevPos;
        QPointF aimDir;
        quint16 hp = 0;
        quint16 maxHp = 0;
        QString nickname;
        quint8 colorId = 255;
        quint8 activeItemKind = 0;
        quint8 activeResourceType = 0;
        quint32 lastTick = 0;
        bool initialized = false;
    };

    QHash<quint32, RemotePlayerNetState> m_remotePlayers;

    bool m_applyingRemoteTile = false;

    static quint32 remoteEntityIdFromPlayerId(quint32 playerId);
    static QColor uiColorFromColorId(quint8 colorId);
    static QPixmap buildKatanaSlotIndicatorPixmap();
    static QPixmap buildResourceSlotIndicatorPixmap(quint8 resourceTileType);
    void appendRemotePlayersToSnapshot();

    //scene
    QGraphicsScene* m_scene = nullptr;

    enum class DarkOverlayMode { None, FadeOut, FadeIn };
    DarkOverlayMode m_darkMode = DarkOverlayMode::None;
    QGraphicsRectItem* m_darkOverlay = nullptr;
    float m_darkAmount = 1.0f;

    QTimer* m_timer = nullptr;

    QLabel* m_timerLabel;
    QString m_timerBaseStyle;

    GameTimer* m_gameTimer = nullptr;
    bool m_gamePaused = true;

    bool m_useNetTimer = false;
    bool m_haveNetTimerSync = false;
    quint8 m_netTimerPhase = 0;
    quint32 m_netTimerMsLeftAtSync = 0;
    qint64 m_netTimerLocalMsAtSync = 0;
    quint8 m_netTimerLastDrawPhase = 255;
    int m_netTimerLastDrawSeconds = -1;
    QElapsedTimer m_netTimerLocalClock;

    //ui
    PlayerSlotWidget* m_slotWidget = nullptr;
    MiniMapWidget* m_minimapWidget = nullptr;

    //camera
    QPointF m_cameraPos;

    QGraphicsPixmapItem* m_buildPreview = nullptr;
    Tile::TileType m_previewType = Tile::TileType::Empty;
    QPoint m_previewTile {-1, -1};

    QHash<quint32, EntityUi> m_entityItems;
    QHash<QPoint, QGraphicsPixmapItem*> m_tileItems;
    QHash<const PickupItem*, QGraphicsPixmapItem*> m_pickupItems;

    QMap<unsigned short, MoveDirection> m_moveKeyMap;
    QMap<Qt::Key, unsigned short> m_slotKeyMap;

    float m_deltaTime = 0.016f;

    float m_animTime = 0.0f;

    double m_scaleSize = 1.25;

    //initialization
    void initDarkOverlay();
    void initGameTimer();
    void initTimerUi();
    void initSlotWidget();
    void initMiniMapWidget();
    void initBuildPreview();
    void initEntitiesUi();

    //map / pickups
    void buildMap();
    void createPickupUi(const PickupItem*);

    //input
    void handleKeyEvent(QKeyEvent*, bool);

    //camera / overlays
    void updateCamera();
    void updateDarkOverlayRect();
    void updateDarkOverlay();

    //ui ticks
    void updateSlotWidget();
    void updateEntitiesUi();
    void updatePickupsUi();
    void updateBuildPreview();

    //entity ui creation
    void initHpBar(EntityUi&);
    void initNameTag(EntityUi&);
    void initAttackIndicator(EntityUi&);
    void initSlotIndicator(EntityUi&);
    void initDamageEffect(EntityUi&);

    void initEntityBodyVisual(const EntitySnapshot&, EntityUi&);
    void initEntitySpritesAndAnimations(const EntitySnapshot&, EntityUi&);

    EntityUi& ensureEntityUi(const EntitySnapshot& snapshotEntity);
    void removeMissingEntityUis(const QSet<quint32>& seenIds);

    //entity ui updates
    void updateSingleEntityUi(const EntitySnapshot& entity, QSet<quint32>& seenIds);
    void updateEntityRotationFromMovement(const QPointF& movementVector, EntityUi& entityUi);
    void updateEntityWalkAnimationFromMovement(const QPointF& movementVector, EntityUi& entityUi, bool allowAnimations);
    void updateEntityAttackAnimation(EntityUi& entityUi, bool allowAnimations);
    void updateEntityBodySprite(EntityUi& entityUi);
    void updateEntityAttackFx(const EntitySnapshot& entity, EntityUi& entityUi);
    void updateDamageFlash(EntityUi& entityUi);
    void startDamageFlashForEntity(quint32 entityId, int damage);
    void updateEntityHp(const EntitySnapshot&, EntityUi&);
    void updateEntityName(const EntitySnapshot&, EntityUi&);
    void updateEntityAtkIndicator(const EntitySnapshot&, EntityUi&);
    void updateEntitySlotIndicator(const EntitySnapshot&, EntityUi&, const QPointF& mouseScene);
    void updateEntitiesVisibility(const EntitySnapshot&, EntityUi&);

    float getRemotePlayerRadius() const;
    void ensureLocalPlayerUi();
    void ensureRemotePlayerUi(quint32 playerId);
    void applyLocalPlayerKnockback(quint32 attackerPlayerId, int appliedDamage);

    void updateSimulationTick();
    void updateSnapshotTick();
    void publishLocalPlayerStateTick();
    void updateUiTick();

private slots:
    void onTick();
    void updateTile(int tileX, int tileY);

    void onCountdownTick(int);
    void onGameStarted();
    void onGameTimerTick(int);
    void onGameEnded();
    void onEnemyHit(const HitInfo& hit);
};

#endif // GAMEVIEW_H
