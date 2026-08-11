#include "Model/Block/CoinBlock.h"

#include "Model/Character.h"
#include "Model/Core/World.h"
#include "Model/Item/Coin.h"
#include "Model/Item/FireFlower.h"
#include "Model/Item/Mushroom.h"
#include "Model/Item/Starman.h"
#include "Model/Map/TileMap.h"

#include <random>

#include "Model/Core/GameManager.h"

#include <algorithm>
#include <string>

namespace model {

namespace {
// Uniform random float in [0, 1). The engine is seeded once per program run so every
// bump has a fresh draw instead of replaying the same sequence.
float randomChance() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
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
    // Dispatched when the player's top face bumps this block's bottom. A block is spent
    // exactly once; afterwards it stays as a plain used block.
    if (!coinAvailable) return;

    coinAvailable = false;
    coinPopElapsed = 0.0f;  // start the pop-out animation
    startBounce();

    // Reward is rolled once per bump. Spawned items rise out of the block's top face; the
    // world queues them so the running update loop is never invalidated.
    const Vector2 spawnPos{getPosition().x,
                           getPosition().y - static_cast<float>(TileMap::TileHeight)};
    const float roll = randomChance();

    // A mushroom walks away from the side the player bumped from. Direction is a Character
    // capability, and only a Character can bump a block, so ask for it as one rather than
    // widening Entity or casting to the concrete Player type.
    const auto* bumper = dynamic_cast<const Character*>(&event.player);
    const int bumpDirection = bumper ? bumper->getDirection() : 1;

    if (roll < MushroomChance) {
        if (world) world->spawn(std::make_unique<Mushroom>(spawnPos, bumpDirection));
    } else if (roll < MushroomChance + FlowerChance) {
        if (world) world->spawn(std::make_unique<FireFlower>(spawnPos));
    } else if (roll < MushroomChance + FlowerChance + StarmanChance) {
        if (world) world->spawn(std::make_unique<Starman>(spawnPos));
    } else {
        // Plain coin. Credited here and now rather than when the sprite is touched — the
        // coin is never in doubt, and the player is underneath the block, not where the
        // coin pops to. The spawned Coin is only the flourish.
        GameManager::instance().addCoin();
        GameManager::instance().addScore(CoinScore);
        if (world) world->spawn(std::make_unique<Coin>(spawnPos));
    }
}

}
