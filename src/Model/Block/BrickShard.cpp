#include "Model/Block/BrickShard.h"

namespace model {

BrickShard::BrickShard(Vector2 position, int quadrant, Vector2 launchVelocity)
    : Character(position, Vector2{8.0f, 8.0f}), quadrant(quadrant) {
    // Purely decorative: no entity pair should ever resolve against it (mirrors Coin).
    hitbox.isTrigger = true;
    setVelocity(launchVelocity);
}

int BrickShard::getQuadrant() const {
    return quadrant;
}

}
