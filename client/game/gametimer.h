#ifndef GAMETIMER_H
#define GAMETIMER_H

#include <QObject>

class GameTimer : public QObject
{
    Q_OBJECT
public:
    explicit GameTimer(QObject* parent = nullptr);

    void startCountdown(int seconds);
    void startGameTimer(int seconds);
    void update(float deltaTime);

    int timeLeft() const;
    bool isCountdown() const;
    bool isGameOn() const;

signals:
    void countdownStarted();
    void countdownTick(int left);
    void gameStarted();
    void gameEnded();
    void gameTick(int left);

private:
    float m_time;
    int m_lastWholeSecond = -1;

    enum class State { Idle, Countdown, Game } m_state;
};

#endif // GAMETIMER_H
