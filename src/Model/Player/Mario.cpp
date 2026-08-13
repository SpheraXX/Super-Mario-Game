#include "Model/Player/Mario.h"

namespace model {

// Sizes are in world units: one world tile is 16x16, and small Mario is exactly one tile.
// (The sprite's source frame is 16x32 pixels and is drawn 1:1 by the renderer.)
Mario::Mario(Vector2 position)
    : Player(position, {16.0f, 16.0f}) {
    // The sprite is one tile wide, but the collision box is shrunk by 4 world units on each
    // side: the artwork sits inside its 16x16 frame with clear space either side, so a
    // full-width box made side grazes far stricter than the art suggests. The box stays
    // centred (offset +4), so bump detection lines up with the sprite. Height is the full
    // small-form tile; Player::update rewrites it on growing (see Player::SmallHeight).
    hitbox = Hitbox({4.0f, 0.0f}, 8.0f, 16.0f, false, CollisionLayer::Player);
}

float Mario::getWalkSpeed() const {
    return WalkSpeed;
}

float Mario::getRunSpeed() const {
    return RunSpeed;
}

float Mario::getMaxJumpSpeed() const {
    return MaxJumpSpeed;
}

float Mario::getJumpAccel() const {
    return JumpAccel;
}

}
