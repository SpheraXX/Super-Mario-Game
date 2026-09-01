#ifndef VIEW_EFFECT_LERPANIMATOR_H
#define VIEW_EFFECT_LERPANIMATOR_H

#include <cmath>
#include <algorithm>

namespace view::effect {

enum class Easing { 
    Linear, 
    OutQuad, 
    OutCubic, 
    InOutSine 
};

class LerpAnimator {
public:
    LerpAnimator() = default;

    LerpAnimator(float from, float to, float duration, Easing easing = Easing::OutQuad)
        : m_from(from), m_to(to), m_duration(duration), m_easing(easing), m_currentValue(from) {
        if (m_duration <= 0.0f) {
            m_duration = 0.001f; // Prevent division by zero
        }
    }

    void update(float dt) {
        if (m_isDone) return;
        
        m_timer += dt;
        if (m_timer >= m_duration) {
            m_timer = m_duration;
            m_isDone = true;
        }

        float t = m_timer / m_duration;
        float easedT = applyEasing(t);
        
        m_currentValue = m_from + (m_to - m_from) * easedT;
    }

    float value() const {
        return m_currentValue;
    }

    bool isDone() const {
        return m_isDone;
    }

    void reset() {
        m_timer = 0.0f;
        m_isDone = false;
        m_currentValue = m_from;
    }

private:
    float applyEasing(float t) const {
        switch (m_easing) {
            case Easing::Linear:
                return t;
            case Easing::OutQuad:
                return t * (2.0f - t);
            case Easing::OutCubic: {
                float inv = 1.0f - t;
                return 1.0f - inv * inv * inv;
            }
            case Easing::InOutSine:
                return -0.5f * (std::cos(3.14159265f * t) - 1.0f);
            default:
                return t;
        }
    }

    float m_from = 0.0f;
    float m_to = 0.0f;
    float m_duration = 1.0f;
    Easing m_easing = Easing::OutQuad;

    float m_timer = 0.0f;
    float m_currentValue = 0.0f;
    bool m_isDone = false;
};

} // namespace view::effect

#endif
