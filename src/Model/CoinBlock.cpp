#include "Model/CoinBlock.h"

namespace model {

CoinBlock::CoinBlock(Vector2 position, Vector2 size)
    : Block(position, size, 'C'), coinAvailable(true) {
    // Solid block: the default hitbox (from Entity) is already full-size, but be
    // explicit so the block always participates in entity-vs-entity collisions.
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

bool CoinBlock::hasCoin() const {
    return coinAvailable;
}

void CoinBlock::collectCoin() {
    coinAvailable = false;
}

}
