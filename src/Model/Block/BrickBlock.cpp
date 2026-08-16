#include "Model/Block/BrickBlock.h"

#include "Model/Block/BrickShard.h"
#include "Model/Core/GameManager.h"
#include "Model/Core/World.h"
#include "Model/Map/TileMap.h"

namespace model {

namespace {
// Toss velocities for the four shards (top pair higher and wider, bottom pair shorter),
// paired with BrickShard's own quadrant numbering (0=TL, 1=TR, 2=BL, 3=BR).
constexpr Vector2 ShardLaunch[4] = {
    {-60.0f, -260.0f},
    {60.0f, -260.0f},
    {-40.0f, -180.0f},
    {40.0f, -180.0f},
};
}

BrickBlock::BrickBlock(Vector2 position, Vector2 size)
    : Block(position, size, '#') {
    // Solid block: same full-size hitbox as every other block.
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

bool BrickBlock::onBlockHit(const BlockHitEvent& event) {
    // The bumper decides whether it is big enough: canBreakBricks() is the Entity-level
    // capability (a big Player), so no type check is needed here. A smash destroys the
    // brick outright — the entity pass skips inactive entities (so it can be walked and
    // jumped through), and the cell is erased from the static map (so nothing lands on
    // the brick's old spot; see isGroundTile in CollisionManager). A small bumper just
    // makes the brick bounce.
    if (event.player.canBreakBricks()) {
        GameManager::instance().addScore(BreakScore);
        if (world) {
            const Vector2 pos = getPosition();
            const Vector2 half{getSize().x / 2.0f, getSize().y / 2.0f};
            for (int quadrant = 0; quadrant < 4; ++quadrant) {
                const Vector2 shardPos{pos.x + (quadrant % 2) * half.x,
                                       pos.y + (quadrant / 2) * half.y};
                world->spawn(std::make_unique<BrickShard>(shardPos, quadrant,
                                                           ShardLaunch[quadrant]));
            }
        }
        eraseFromMap();
        isActive = false;
        return true;
    }
    startBounce();
    return true;
}

void BrickBlock::eraseFromMap() {
    // The block was built at tileOrigin(row, column), so the cell inverse is exact.
    // Rows are stored top-down in the map file, which is why the flip appears here too.
    if (!world) return;
    const auto column = static_cast<std::size_t>(getPosition().x / TileMap::TileWidth);
    const auto row = TileMap::Rows - 1 - static_cast<std::size_t>(getPosition().y / TileMap::TileHeight);
    world->removeTile(row, column);
}

}
