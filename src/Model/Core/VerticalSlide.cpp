#include "Model/Core/VerticalSlide.h"
#include "Model/Entity.h"

#include <cmath>

namespace model {

void VerticalSlide::begin(float start, float target, float speed, Axis axis) {
    this->target = target;
    this->speed = std::fabs(speed);
    this->axis = axis;
    direction = target >= start ? 1.0f : -1.0f;
    done = false;
}

bool VerticalSlide::isDone() const {
    return done;
}

bool VerticalSlide::advance(Entity& entity, float deltaTime) {
    if (done) {
        return false;
    }

    const Vector2 position = entity.getPosition();
    const float current = axis == Axis::Vertical ? position.y : position.x;
    const float next = current + direction * speed * deltaTime;
    const bool passedTarget = direction > 0.0f ? next >= target : next <= target;
    const float resolved = passedTarget ? target : next;
    entity.setPosition(axis == Axis::Vertical ? Vector2{position.x, resolved}
                                               : Vector2{resolved, position.y});
    if (passedTarget) {
        done = true;
        return false;
    }
    return true;
}

}
