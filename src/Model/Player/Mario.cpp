#include "Model/Player/Mario.h"

namespace model {

// Sizes are in world units: one world tile is 32x32, and small Mario is exactly one tile.
// (The sprite's source frame is 16x32 pixels and gets scaled up 2x by the renderer.)
Mario::Mario(Vector2 position)
    : Player(position, {32.0f, 32.0f}) {
}

float Mario::getWalkSpeed() const {
    return WalkSpeed;
}

float Mario::getRunSpeed() const {
    return RunSpeed;
}

float Mario::getJumpForce() const {
    return JumpForce;
}

}
