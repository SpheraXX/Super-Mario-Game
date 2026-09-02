#include "Controller/LevelCompletion.h"

#include "Controller/LevelGeometry.h"
#include "Model/Entity.h"
#include "Model/Level/FlagPole.h"

namespace controller {

void LevelCompletion::clear() {
    flagPolePtr = nullptr;
}

void LevelCompletion::build(
    model::TileMap& map,
    std::vector<std::unique_ptr<model::Entity>>& entities) {
    // (Guard: with a failed load columns is 0 and there is nothing to spawn.)
    const std::size_t columns = map.getColumns();
    if (columns < LevelPaddingTiles) {
        return;
    }
    const std::size_t rows = map.getRows();
    const std::size_t tileWidth = model::TileMap::TileWidth;
    const std::size_t tileHeight = model::TileMap::TileHeight;
    const std::size_t baseColumns = columns - LevelPaddingTiles;
    const float groundTop = geometry::groundTopAt(map, baseColumns > 0 ? baseColumns - 1 : 0);
    const float poleHeight = 112.0f;

    // The goal castle is painted into the grid using CastleUpperSymbol and CastleLowerSymbol
    // standing on the ground: upper tower is 3x2, lower base is 5x3.
    const std::size_t groundRowTop =
        rows - 1 - static_cast<std::size_t>(groundTop / static_cast<float>(tileHeight));
    const std::size_t castleCol = baseColumns + CastleOffsetTiles;

    if (groundRowTop + 5 < rows && castleCol + 4 < columns) {
        map.setTile(groundRowTop + 5, castleCol + 1, model::TileMap::CastleUpperSymbol);
        map.setTile(groundRowTop + 3, castleCol, model::TileMap::CastleLowerSymbol);
    }

    auto flag = std::make_unique<model::FlagPole>(
        model::Vector2{static_cast<float>((baseColumns + PoleOffsetTiles) * tileWidth),
                       groundTop - poleHeight},
        model::Vector2{4.0f, poleHeight});
    flagPolePtr = flag.get();
    entities.push_back(std::move(flag));
}

bool LevelCompletion::isTouched() const {
    return flagPolePtr && flagPolePtr->isTouched();
}

model::FlagPole* LevelCompletion::flagPole() const {
    return flagPolePtr;
}

float LevelCompletion::castleDoorX(const model::TileMap& map) const {
    return static_cast<float>(
        (map.getColumns() - LevelPaddingTiles + CastleOffsetTiles + 2) * model::TileMap::TileWidth);
}

}