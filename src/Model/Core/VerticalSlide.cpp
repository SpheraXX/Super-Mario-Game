#include "Model/Core/VerticalSlide.h"
#include "Model/Entity.h"

#include <cmath>

namespace model {

void VerticalSlide::begin(float startY, float targetY, float speed) {
    this->targetY = targetY;
    this->speed = std::fabs(speed);
    direction = targetY >= startY ? 1.0f : -1.0f;
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
    const float nextY = position.y + direction * speed * deltaTime;
    const bool passedTarget = direction > 0.0f ? nextY >= targetY : nextY <= targetY;
    if (passedTarget) {
        entity.setPosition({position.x, targetY});
        done = true;
        return false;
    }
    entity.setPosition({position.x, nextY});
    return true;
}

}
