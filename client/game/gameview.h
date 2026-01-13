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

private:
    Game m_game;

    QGraphicsScene* m_scene;
    QGraphicsPixmapItem* m_playerItem;
    QTimer* m_timer;

    QMap<Qt::Key, MoveDirection> m_keyMap;

    float deltaTime = 0.016f;

    bool m_up = false;
    bool m_down = false;
    bool m_left = false;
    bool m_right = false;

    // helper
    void handleKeyEvent(QKeyEvent*, bool);

private slots:
    void onTick();
};

#endif // GAMEVIEW_H
