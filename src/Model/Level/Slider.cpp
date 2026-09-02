#include "Model/Level/Slider.h"

#include "Model/Core/Hitbox.h"

#include <cmath>

namespace model {

Slider::Slider(Vector2 position, Vector2 size, Axis axis, float travelDistance, float speed)
    : Entity(position, size),
      origin(position),
      axis(axis),
      travelDistance(travelDistance),
      speed(speed) {
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

void Slider::update(float deltaTime) {
    totalTime += deltaTime;
    const Vector2 before = getPosition();

    if (travelDistance <= 0.0f || speed <= 0.0f) {
        lastDelta = {0.0f, 0.0f};
        return;
    }

    // Pure function of total elapsed time rather than an incremental bounce: a triangle
    // wave of period 2*travelDistance sidesteps every edge case an iterative "step, then
    // flip direction at the end" loop has (drift, and infinite-looping on a zero-length
    // travel) without needing a guard for any of them.
    const float period = 2.0f * travelDistance;
    float cyclePos = std::fmod(totalTime * speed, period);
    if (cyclePos < 0.0f) {
        cyclePos += period;
    }
    const float offset = (cyclePos <= travelDistance) ? cyclePos : (period - cyclePos);

    Vector2 newPos = origin;
    if (axis == Axis::Horizontal) {
        newPos.x += offset;
    } else {
        newPos.y += offset;
    }
    lastDelta = {newPos.x - before.x, newPos.y - before.y};
    setPosition(newPos);
}

}
