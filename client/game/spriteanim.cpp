#include "spriteanim.h"

SpriteAnim::SpriteAnim(QVector<QPixmap> frames, float fps, bool loop)
    : m_frames(std::move(frames))
    , m_fps(fps)
    , m_loop(loop)
{
    if (m_fps <= 0.0f) m_fps = 1.0f;
}

void SpriteAnim::setFrames(QVector<QPixmap> frames) {
    m_frames = std::move(frames);
    reset();
}
const QVector<QPixmap>& SpriteAnim::getFrames() const { return m_frames; }

void SpriteAnim::setFps(float fps) { m_fps = (fps > 0.0f) ? fps : 1.0f; }
float SpriteAnim::getFps() const { return m_fps; }

void SpriteAnim::setLoop(bool loop) { m_loop = loop; }
bool SpriteAnim::getLoop() const { return m_loop; }

void SpriteAnim::reset() {
    m_time = 0.0f;
    m_playing = true;
}

void SpriteAnim::stop() { m_playing = false; }
void SpriteAnim::play() { m_playing = true; }
bool SpriteAnim::isPlaying() const { return m_playing; }

quint16 SpriteAnim::frameIndexForTime(float time) const {
    if (m_frames.isEmpty()) return 0;

    const int framesSize = m_frames.size();
    int index = int(time * m_fps);

    if (m_loop) {
        if (framesSize > 0) index %= framesSize;
        return index;
    }

    return quint16(std::clamp(index, 0, framesSize - 1));
}

const QPixmap& SpriteAnim::tick(float dt) {
    static const QPixmap nullPx;
    if (m_frames.isEmpty()) return nullPx;

    if (!m_playing) return currentFrame();

    if (dt < 0.0f) dt = 0.0f;

    if (!m_loop && finished()) return currentFrame();

    m_time += dt;

    if (!m_loop) {
        const float endTime = float(m_frames.size() - 1) / m_fps;
        if (m_time > endTime) m_time = endTime;
    }

    return currentFrame();
}

const QPixmap& SpriteAnim::currentFrame() const {
    static const QPixmap nullPx;
    if (m_frames.isEmpty()) return nullPx;

    const unsigned short index = frameIndexForTime(m_time);
    return m_frames[index];
}

quint16 SpriteAnim::currentIndex() const {
    if (m_frames.isEmpty()) return -1;
    return frameIndexForTime(m_time);
}

bool SpriteAnim::finished() const {
    if (m_loop) return false;
    if (m_frames.isEmpty()) return true;

    const unsigned short index = frameIndexForTime(m_time);
    return index >= (m_frames.size() - 1);
}

QVector<QPixmap> SpriteAnim::sliceStrip(const QPixmap& strip, quint16 frameCount, bool horizontal) {
    QVector<QPixmap> out;
    if (strip.isNull()) return out;
    if (frameCount <= 0) return out;

    out.reserve(frameCount);

    if (horizontal) {
        const int width = strip.width() / frameCount;
        const int height = strip.height();
        for (int index = 0; index < frameCount; ++index)
            out.push_back(strip.copy(index * width, 0, width, height));
    } else {
        const int width = strip.width();
        const int height = strip.height() / frameCount;
        for (int index = 0; index < frameCount; ++index)
            out.push_back(strip.copy(index * width, 0, width, height));
    }

    return out;
}
