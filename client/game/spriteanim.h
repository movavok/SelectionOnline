#ifndef SPRITEANIM_H
#define SPRITEANIM_H

#include <QPixmap>
#include <QVector>
#include <algorithm>

class SpriteAnim
{
public:
    SpriteAnim() = default;
    explicit SpriteAnim(QVector<QPixmap>, float fps = 12.0f, bool loop = true);

    void setFrames(QVector<QPixmap>);
    const QVector<QPixmap>& getFrames() const;

    void setFps(float fps);
    float getFps() const;

    void setLoop(bool loop);
    bool getLoop() const;

    void reset();
    void stop();
    void play();
    bool isPlaying() const;

    const QPixmap& tick(float);
    const QPixmap& currentFrame() const;

    quint16 currentIndex() const;
    bool finished() const;

    static QVector<QPixmap> sliceStrip(const QPixmap& strip, quint16 frameCount, bool horizontal = true);

private:
    QVector<QPixmap> m_frames;
    float m_fps = 12.0f;
    float m_time = 0.0f;
    bool m_loop = true;
    bool m_playing = true;

    quint16 frameIndexForTime(float) const;
};

#endif // SPRITEANIM_H
