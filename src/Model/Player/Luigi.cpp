#include "Model/Player/Luigi.h"

namespace model {

// World units, same as Mario: one world tile tall.
Luigi::Luigi(Vector2 position)
    : Player(position, {16.0f, 16.0f}) {
    // Same tightened box as Mario: shrunk by 4 world units per side.
    hitbox = Hitbox({4.0f, 0.0f}, 8.0f, 16.0f, false, CollisionLayer::Player);
}

float Luigi::getWalkSpeed() const {
    return WalkSpeed;
}

float Luigi::getRunSpeed() const {
    return RunSpeed;
}

float Luigi::getMaxJumpSpeed() const {
    return MaxJumpSpeed;
}

float Luigi::getJumpAccel() const {
    return JumpAccel;
}

}
