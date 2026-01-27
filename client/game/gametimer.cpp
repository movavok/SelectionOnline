#include "gametimer.h"

GameTimer::GameTimer(QObject* parent)
    : QObject(parent)
    , m_time(0.0f)
    , m_state(State::Idle) {}

void GameTimer::startCountdown(int seconds) {
    m_time = float(seconds);
    m_lastWholeSecond = -1;
    m_state = State::Countdown;

    emit countdownStarted();
    emit countdownTick(seconds);
}

void GameTimer::startGameTimer(int seconds) {
    m_time = float(seconds);
    m_lastWholeSecond = -1;
    m_state = State::Game;

    emit gameTick(int(std::ceil(m_time)));
}

void GameTimer::update(float deltaTime) {
    if (m_state == State::Idle) return;

    m_time -= deltaTime;

    if (m_time <= 0.0f) {
        m_time = 0.0f;

        if (m_state == State::Countdown) {
            m_state = State::Idle;
            emit gameStarted();
        }
        else if (m_state == State::Game) {
            m_state = State::Idle;
            emit gameEnded();
        }
    } else {
        int whole = int(std::ceil(m_time));
        if (whole != m_lastWholeSecond) {
            m_lastWholeSecond = whole;
            if (m_state == State::Countdown) emit countdownTick(whole);
            if (m_state == State::Game) emit gameTick(whole);
        }
    }
}

int GameTimer::timeLeft() const {
    return int(std::ceil(m_time));
}

bool GameTimer::isCountdown() const { return m_state == State::Countdown; }

bool GameTimer::isGameOn() const { return m_state == State::Game; }

