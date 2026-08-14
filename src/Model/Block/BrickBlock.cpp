#include "Model/Block/BrickBlock.h"

#include "Model/Core/GameManager.h"
#include "Model/Core/World.h"
#include "Model/Map/TileMap.h"

namespace model {

BrickBlock::BrickBlock(Vector2 position, Vector2 size)
    : Block(position, size, '#') {
    // Solid block: same full-size hitbox as every other block.
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

void BrickBlock::onBlockHit(const BlockHitEvent& event) {
    // The bumper decides whether it is big enough: canBreakBricks() is the Entity-level
    // capability (a big Player), so no type check is needed here. A smash destroys the
    // brick outright — the entity pass skips inactive entities (so it can be walked and
    // jumped through), and the cell is erased from the static map (so nothing lands on
    // the brick's old spot; see isGroundTile in CollisionManager). A small bumper just
    // makes the brick bounce.
    if (event.player.canBreakBricks()) {
        GameManager::instance().addScore(BreakScore);
        eraseFromMap();
        isActive = false;
        return;
    }
    startBounce();
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
