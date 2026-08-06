#include "Model/Player/Mario.h"

namespace model {

// Sizes are in world units: one world tile is 32x32, and small Mario is exactly one tile.
// (The sprite's source frame is 16x32 pixels and gets scaled up 2x by the renderer.)
Mario::Mario(Vector2 position)
    : Player(position, {32.0f, 32.0f}) {
    // The sprite is one tile wide, but the collision box is shrunk by 4 source pixels
    // (8 world units) on each side: the sprite's artwork sits inside a 16x16 source
    // frame, so a full-width box made side grazes far more strict than the art suggests.
    // The box stays centred (offset +8), so bump detection lines up with the sprite.
    hitbox = Hitbox({8.0f, 0.0f}, 16.0f, 32.0f, false, CollisionLayer::Player);
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
