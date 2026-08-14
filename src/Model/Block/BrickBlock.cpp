#include "Model/Block/BrickBlock.h"

#include "Model/Core/GameManager.h"

namespace model {

BrickBlock::BrickBlock(Vector2 position, Vector2 size)
    : Block(position, size, '#') {
    // Solid block: same full-size hitbox as every other block.
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

void BrickBlock::onBlockHit(const BlockHitEvent& event) {
    // The bumper decides whether it is big enough: canBreakBricks() is the Entity-level
    // capability (a big Player), so no type check is needed here. A smash destroys the
    // brick outright — the level's update/collision/render passes all skip inactive
    // entities, which is how the brick leaves the world. A small bumper just makes it
    // bounce.
    if (event.player.canBreakBricks()) {
        GameManager::instance().addScore(BreakScore);
        isActive = false;
        return;
    }
    startBounce();
}

}
