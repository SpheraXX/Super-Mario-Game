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
    const float poleHeight = 224.0f;

    // The goal castle is painted into the grid from its 21-tile sheet (see
    // TileMap::CastleSymbols), row-major over a 5x5 silhouette standing on the ground:
    // the upper two rows are the 3-wide tower, the lower three the 5-wide base, the
    // centre-bottom pair is the door, and the two outer cells of the tower rows stay
    // air. The paint is deterministic, so re-running resetLevel (enter/death) is
    // idempotent.
    const std::size_t groundRowTop =
        rows - 1 - static_cast<std::size_t>(groundTop / static_cast<float>(tileHeight));
    const std::size_t castleCol = baseColumns + CastleOffsetTiles;
    std::size_t castleIndex = 0;
    for (std::size_t silhouetteRow = 0; silhouetteRow < 5; ++silhouetteRow) {
        for (std::size_t silhouetteColumn = 0; silhouetteColumn < 5; ++silhouetteColumn) {
            if (silhouetteRow < 2 && (silhouetteColumn == 0 || silhouetteColumn == 4)) {
                continue;
            }
            map.setTile(groundRowTop + 5 - silhouetteRow, castleCol + silhouetteColumn,
                        model::TileMap::CastleSymbols[castleIndex++]);
        }
    }

    auto flag = std::make_unique<model::FlagPole>(
        model::Vector2{static_cast<float>((baseColumns + PoleOffsetTiles) * tileWidth),
                       groundTop - poleHeight},
        model::Vector2{8.0f, poleHeight});
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
        (map.getColumns() - LevelPaddingTiles + CastleOffsetTiles) * model::TileMap::TileWidth);
}

}