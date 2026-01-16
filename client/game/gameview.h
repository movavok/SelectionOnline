#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QObject>
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
    Game m_game;

    //scene
    QGraphicsScene* m_scene = nullptr;
    QTimer* m_timer = nullptr;

    //camera
    QPointF m_cameraPos;

    QGraphicsEllipseItem* m_playerItem = nullptr;

    QGraphicsPathItem* m_attackIndicator = nullptr;

    QPointF m_mouseScenePos;

    //hp bar
    QGraphicsRectItem* m_hpBack = nullptr;
    QGraphicsRectItem* m_hpFill = nullptr;
    QGraphicsRectItem* m_hpTextMask = nullptr;
    QGraphicsTextItem* m_hpTextWhite = nullptr;
    QGraphicsTextItem* m_hpTextBlack = nullptr;

    QMap<Qt::Key, MoveDirection> m_keyMap;

    float deltaTime = 0.016f;

    bool m_up = false;
    bool m_down = false;
    bool m_left = false;
    bool m_right = false;

    //helper
    void initPlayerUi();
    void initHpBar();
    void initAttackIndicator();
    void buildMap();

    void handleKeyEvent(QKeyEvent*, bool);

    void updateCamera();
    void updateHpBar();
    void updateAttackIndicator();

private slots:
    void onTick();
};

#endif // GAMEVIEW_H
