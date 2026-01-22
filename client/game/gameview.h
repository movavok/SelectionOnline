#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QTimer>
#include <cstdlib>
#include <ctime>

#include "../input/inputtypes.h"
#include "game.h"

class GameView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GameView(QWidget* parent = nullptr);

    void useMovementScheme(MovementScheme);
    void setupSlotKeys();

protected:
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

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
    QTimer* m_timer = nullptr;

    //camera
    QPointF m_cameraPos;

    QGraphicsPixmapItem* m_buildPreview = nullptr;
    Tile::TileType m_previewType = Tile::TileType::Empty;
    QPoint m_previewTile{-1, -1};

    QHash<Entity*, EntityUi> m_entityItems;
    QHash<QPoint, QGraphicsPixmapItem*> m_tileItems;
    QHash<const PickupItem*, QGraphicsPixmapItem*> m_pickupItems;

    QMap<unsigned short, MoveDirection> m_moveKeyMap;
    QMap<unsigned short, unsigned short> m_slotKeyMap;

    float m_deltaTime = 0.016f;

    double m_scaleSize = 1.25;

    //helper
    void initEntitiesUi();
    void initHpBar(EntityUi&);
    void initAttackIndicator(EntityUi&);
    void initSlotIndicator(EntityUi&);
    void initBuildPreview();
    void buildMap();
    void createPickupUi(const PickupItem*);

    void handleKeyEvent(QKeyEvent*, bool);

    void updateCamera();
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
};

#endif // GAMEVIEW_H
