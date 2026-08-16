#include "Model/Core/LevelTimer.h"

#include <algorithm>
#include <cmath>

namespace model {

LevelTimer::LevelTimer(float startSeconds)
    : remaining(startSeconds), paused(false) {
}

void LevelTimer::update(float deltaTime) {
    if (paused) return;
    remaining = std::max(0.0f, remaining - deltaTime);
}

void LevelTimer::pause() {
    paused = true;
}

void LevelTimer::resume() {
    paused = false;
}

void LevelTimer::reset(float startSeconds) {
    remaining = startSeconds;
    paused = false;
}

bool LevelTimer::isPaused() const {
    return paused;
}

bool LevelTimer::isExpired() const {
    return remaining <= 0.0f;
}

int LevelTimer::getRemainingSeconds() const {
    return static_cast<int>(std::ceil(remaining - 1e-6f));
}

float LevelTimer::getRemaining() const {
    return remaining;
}

}
