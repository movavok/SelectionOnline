#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsColorizeEffect>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <cstdlib>
#include <ctime>

#include "../input/inputtypes.h"
#include "../ui/playerslotwidget.h"
#include "game.h"
#include "gametimer.h"
#include "hitinfo.h"

class GameView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GameView(QWidget* parent = nullptr);

    void useMovementScheme(MovementScheme);
    void setupSlotKeys();

    void startGameWithCountdown();

protected:
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

    void wheelEvent(QWheelEvent*) override;

    void resizeEvent(QResizeEvent*) override;

private:
    struct EntityUi {
        QGraphicsEllipseItem* body = nullptr;
        QGraphicsPixmapItem* sprite = nullptr;
        QGraphicsColorizeEffect* damageEffect = nullptr;
        float damageFlashRemaining = 0.0f;
        float damageFlashTotal = 0.0f;
        QGraphicsRectItem* hpBack = nullptr;
        QGraphicsRectItem* hpFill = nullptr;
        QGraphicsRectItem* hpTextMask = nullptr;
        QGraphicsTextItem* hpTextWhite = nullptr;
        QGraphicsTextItem* hpTextBlack = nullptr;
        QGraphicsPathItem* attackIndicator = nullptr;
        QGraphicsPixmapItem* slotIndicator = nullptr;
    };

    Game m_game;

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

    //ui
    PlayerSlotWidget* m_slotWidget = nullptr;

    //camera
    QPointF m_cameraPos;

    QGraphicsPixmapItem* m_buildPreview = nullptr;
    Tile::TileType m_previewType = Tile::TileType::Empty;
    QPoint m_previewTile {-1, -1};

    QHash<Entity*, EntityUi> m_entityItems;
    QHash<QPoint, QGraphicsPixmapItem*> m_tileItems;
    QHash<const PickupItem*, QGraphicsPixmapItem*> m_pickupItems;

    QMap<unsigned short, MoveDirection> m_moveKeyMap;
    QMap<unsigned short, unsigned short> m_slotKeyMap;

    float m_deltaTime = 0.016f;

    double m_scaleSize = 1.25;

    //helper
    void initDarkOverlay();

    void initGameTimer();
    void initTimerUi();

    void initSlotWidget();

    void initEntitiesUi();
    void initHpBar(EntityUi&);
    void initAttackIndicator(EntityUi&);
    void initSlotIndicator(EntityUi&);
    void initDamageEffect(EntityUi&);

    void initBuildPreview();
    void buildMap();
    void createPickupUi(const PickupItem*);

    void handleKeyEvent(QKeyEvent*, bool);

    void updateCamera();

    void updateDarkOverlayRect();
    void updateDarkOverlay();

    void updateSlotWidget();

    void updateEntitiesUi();
    void updateEntityHp(Entity*, EntityUi&);
    void updateEntityAtkIndicator(Entity*, EntityUi&);
    void updateEntitySlotIndicator(Entity*, EntityUi&);

    void updateEntitiesVisibility(Entity*, EntityUi&);
    void updatePickupsUi();
    void updateBuildPreview();

private slots:
    void onTick();
    void updateTile(int x, int y);

    void onCountdownTick(int);
    void onGameStarted();
    void onGameTimerTick(int);
    void onGameEnded();
    void onEnemyHit(const HitInfo& hit);
};

#endif // GAMEVIEW_H
