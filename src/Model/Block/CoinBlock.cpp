#include "Model/Block/CoinBlock.h"
#include "Model/Core/GameManager.h"

#include <algorithm>
#include <fstream>
#include <string>

namespace model {

namespace {
// TEMP trace instrumentation (removed after playtest).
void trace(const std::string& msg) {
    std::ofstream out("trace_log.txt", std::ios::app);
    out << msg << '\n';
}
}

CoinBlock::CoinBlock(Vector2 position, Vector2 size)
    : Block(position, size, 'C'),
      coinAvailable(true),
      coinPopElapsed(CoinPopDuration) {
    // Solid block: the default hitbox (from Entity) is already full-size, but be
    // explicit so the block always participates in entity-vs-entity collisions.
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

bool CoinBlock::isOpened() const {
    return !coinAvailable;
}

bool CoinBlock::isCoinPopping() const {
    return coinPopElapsed < CoinPopDuration;
}

float CoinBlock::getCoinPopProgress() const {
    return coinPopElapsed / CoinPopDuration;
}

void CoinBlock::update(float deltaTime) {
    Block::update(deltaTime);
    if (coinPopElapsed < CoinPopDuration) {
        coinPopElapsed = std::min(coinPopElapsed + deltaTime, CoinPopDuration);
    }
}

void CoinBlock::onBlockHit(const BlockHitEvent& event) {
    (void)event;
    // Dispatched when the player's top face bumps this block's bottom. The coin is
    // collected once; afterwards the block stays as a plain used block.
    if (coinAvailable) {
        coinAvailable = false;
        coinPopElapsed = 0.0f;  // start the pop-out animation
        GameManager::instance().addScore(200);
        trace("coinCollected score=" + std::to_string(GameManager::instance().getScore()));
        startBounce();
    }
}

}
