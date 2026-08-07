#include "Model/Level/FlagPole.h"

#include "Model/Core/Hitbox.h"

namespace model {

FlagPole::FlagPole(Vector2 position, Vector2 size)
    : Entity(position, size) {
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, true, CollisionLayer::Trigger);
}

void FlagPole::onTriggerEnter(Entity& other) {
    if (other.hitbox.layer == CollisionLayer::Player) {
        touched = true;
    }
}

bool FlagPole::isTouched() const {
    return touched;
}

}
