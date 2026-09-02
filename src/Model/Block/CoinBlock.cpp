#include "Model/Block/CoinBlock.h"

#include "Model/Character.h"
#include "Model/Core/World.h"
#include "Model/Item/Coin.h"
#include "Model/Item/FireFlower.h"
#include "Model/Item/Mushroom.h"
#include "Model/Item/Starman.h"

#include <random>

#include "Model/Core/GameManager.h"

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
      coinAvailable(true) {
    // Solid block: the default hitbox (from Entity) is already full-size, but be
    // explicit so the block always participates in entity-vs-entity collisions.
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

bool CoinBlock::isOpened() const {
    return !coinAvailable;
}

bool CoinBlock::onBlockHit(const BlockHitEvent& event) {
    // Dispatched when the player's top face bumps this block's bottom. A block is spent
    // exactly once; afterwards it stays as a plain used block — the false return stops
    // the bump from counting at all, so it acts like a G block (no bounce, and nothing
    // standing on top reacts).
    if (!coinAvailable) return false;

    coinAvailable = false;
    startBounce();

    // Reward is rolled once per bump. Every reward comes out of the block's own cell:
    // the item is spawned inside it and rises through the block face via ItemEmergence
    // (the coin pops on its own physics). The world queues everything so the running
    // update loop is never invalidated.
    const float roll = randomChance();

    // A mushroom walks away from the side the player bumped from. Direction is a Character
    // capability, and only a Character can bump a block, so ask for it as one rather than
    // widening Entity or casting to the concrete Player type.
    const auto* bumper = dynamic_cast<const Character*>(&event.player);
    const int bumpDirection = bumper ? bumper->getDirection() : 1;

    if (roll < MushroomChance) {
        if (world) {
            auto mushroom = std::make_unique<Mushroom>(getPosition(), bumpDirection);
            mushroom->beginEmergence(getPosition(), getSize());
            world->spawn(std::move(mushroom));
            if (auto audio = GameManager::instance().getAudioDelegate()) audio->playSound("11. Item");
        }
    } else if (roll < MushroomChance + FlowerChance) {
        if (world) {
            auto flower = std::make_unique<FireFlower>(getPosition());
            flower->beginEmergence(getPosition(), getSize());
            world->spawn(std::move(flower));
            if (auto audio = GameManager::instance().getAudioDelegate()) audio->playSound("11. Item");
        }
    } else if (roll < MushroomChance + FlowerChance + StarmanChance) {
        if (world) {
            auto starman = std::make_unique<Starman>(getPosition());
            starman->beginEmergence(getPosition(), getSize());
            world->spawn(std::move(starman));
            if (auto audio = GameManager::instance().getAudioDelegate()) audio->playSound("11. Item");
        }
    } else {
        // Plain coin. Credited here and now rather than when the sprite is touched — the
        // coin is never in doubt, and the player is underneath the block, not where the
        // coin pops to. The Coin entity is only the flourish: it pops out of the block's
        // own cell and disappears the moment it falls back to its starting height.
        GameManager::instance().addCoin();
        GameManager::instance().addScore(CoinScore);
        if (auto audio = GameManager::instance().getAudioDelegate()) audio->playSound("07. Coin");
        if (world) world->spawn(std::make_unique<Coin>(getPosition()));
    }
    return true;
}

}
