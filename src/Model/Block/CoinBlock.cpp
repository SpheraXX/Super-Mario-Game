#include "Model/Block/CoinBlock.h"
#include "Model/Core/GameManager.h"

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

void CoinBlock::onCollision(Entity& other, CollisionType side) {
    // Bumped from below: the block's bottom face is hit by the player. The coin is
    // collected once; afterwards the block stays as a plain used block.
    if (side == CollisionType::Bottom && other.hitbox.layer == CollisionLayer::Player) {
        if (coinAvailable) {
            collectCoin();
            // A coin counts twice over: score now, and towards the extra-life tally.
            GameManager::instance().addScore(CoinScore);
            GameManager::instance().addCoin();
        }
    }
}

}
