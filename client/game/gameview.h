#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QTimer>

#include "../input/inputtypes.h"
#include "game.h"

class GameView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GameView(QWidget* parent = nullptr);

    void useMovementScheme(MovementScheme);

protected:
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

private:
    struct EntityUi {
        QGraphicsEllipseItem* body = nullptr;
        QGraphicsRectItem* hpBack = nullptr;
        QGraphicsRectItem* hpFill = nullptr;
        QGraphicsRectItem* hpTextMask = nullptr;
        QGraphicsTextItem* hpTextWhite = nullptr;
        QGraphicsTextItem* hpTextBlack = nullptr;
        QGraphicsPathItem* attackIndicator = nullptr;
    };

    Game m_game;

    //scene
    QGraphicsScene* m_scene = nullptr;
    QTimer* m_timer = nullptr;

    //camera
    QPointF m_cameraPos;
    QPointF m_mouseScenePos;

    QHash<Entity*, EntityUi> m_entityItems;
    QHash<QPoint, QGraphicsRectItem*> m_tileItems;
    QMap<Qt::Key, MoveDirection> m_keyMap;

    float deltaTime = 0.016f;

    //helper
    void initEntitiesUi();
    void initHpBar(Entity*, EntityUi&);
    void initAttackIndicator(EntityUi&);
    void buildMap();

    void handleKeyEvent(QKeyEvent*, bool);

    void updateCamera();
    void updateEntitiesUi();
    void updateEntityHp(Entity*, EntityUi&);
    void updateEntityAtkIndicator(Entity*, EntityUi&);

private slots:
    void onTick();
    void updateTile(int x, int y);
};

#endif // GAMEVIEW_H
