#include "Model/Level/LevelGoal.h"

#include "Model/Core/Hitbox.h"

namespace model {

LevelGoal::LevelGoal(Vector2 position, Vector2 size)
    : Entity(position, size) {
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, true, CollisionLayer::Trigger);
}

void LevelGoal::onTriggerEnter(Entity& other) {
    if (other.hitbox.layer == CollisionLayer::Player) {
        touched = true;
    }
}

bool LevelGoal::isTouched() const {
    return touched;
}

}
