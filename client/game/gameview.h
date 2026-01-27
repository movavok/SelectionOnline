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

    void resizeEvent(QResizeEvent*) override;

private:
    struct EntityUi {
        QGraphicsEllipseItem* body = nullptr;
        QGraphicsPixmapItem* sprite = nullptr;
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

    QGraphicsRectItem* m_grayOverlay = nullptr;
    float m_grayAmount = 1.0f;

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
    void initGrayOverlay();

    void initGameTimer();
    void initTimerUi();

    void initSlotWidget();

    void initEntitiesUi();
    void initHpBar(EntityUi&);
    void initAttackIndicator(EntityUi&);
    void initSlotIndicator(EntityUi&);

    void initBuildPreview();
    void buildMap();
    void createPickupUi(const PickupItem*);

    void handleKeyEvent(QKeyEvent*, bool);

    void updateCamera();

    void updateGrayOverlayRect();
    void updateGrayOverlay();

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
};

#endif // GAMEVIEW_H
