#include "Model/Block/CoinBlock.h"
#include "Model/Core/World.h"
#include "Model/Item/FireFlower.h"
#include "Model/Item/Mushroom.h"
#include "Model/Item/Starman.h"
#include "Model/Map/TileMap.h"
#include "Model/Player/Player.h"

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
    // Bumped from below: the block's bottom face is hit by the player.
    if (side != CollisionType::Bottom || other.hitbox.layer != CollisionLayer::Player) return;
    if (!coinAvailable) return;

    // Accept a bump when the player actually overlaps this block by a visible amount.
    // Using the player's centre was too strict at the edge and made valid hits look like
    // "no coin spent", so the renderer stayed on the question-block tile.
    const float playerLeft = other.getPosition().x + other.hitbox.offset.x;
    const float playerRight = playerLeft + other.hitbox.width;
    const float blockLeft = getPosition().x + hitbox.offset.x;
    const float blockRight = blockLeft + hitbox.width;
    const float overlapLeft = std::max(playerLeft, blockLeft);
    const float overlapRight = std::min(playerRight, blockRight);
    const float horizontalOverlap = overlapRight - overlapLeft;
    // A valid AABB contact can be a fraction of a pixel at an edge.  Require only a
    // positive overlap with a small tolerance; a larger arbitrary cutoff makes a block
    // in a narrow corridor respond only when the player is visually centred beneath it.
    constexpr float ContactEpsilon = 0.01f;
    if (horizontalOverlap <= ContactEpsilon) return;

    // Spend the block immediately: the renderer now draws the used-block colour.
    coinAvailable = false;

    auto* player = dynamic_cast<Player*>(&other);
    const float roll = randomChance();

    // Reward is rolled once per bump. Spawned items rise out of the block's top face;
    // the world queues them so the running update loop is never invalidated.
    const Vector2 spawnPos{getPosition().x,
                           getPosition().y - static_cast<float>(TileMap::TileHeight)};

    if (roll < MushroomChance) {
        if (world) {
            world->spawn(std::make_unique<Mushroom>(spawnPos, player ? player->getDirection() : 1));
        }
    } else if (roll < MushroomChance + FlowerChance) {
        if (world) {
            world->spawn(std::make_unique<FireFlower>(spawnPos));
        }
    } else if (roll < MushroomChance + FlowerChance + StarmanChance) {
        if (world) {
            world->spawn(std::make_unique<Starman>(spawnPos));
        }
    } else if (player) {
        // Plain coin: real-Mario scoring — one coin (100 coins = an extra life) + 200 points.
        player->addCoin();
        player->addScore(200);
    }
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
