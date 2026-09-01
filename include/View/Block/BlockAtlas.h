#ifndef VIEW_BLOCK_BLOCKATLAS_H
#define VIEW_BLOCK_BLOCKATLAS_H

#include "Model/World/WorldType.h"

#include <SFML/System/Vector2.hpp>

namespace view {
namespace atlas {

// Where each landscape's block quadrant begins in the shared sheet (super_mario_asset.png).
//
// groundOrigin  — the beveled stone tile (column 0). Used for solid terrain ('G') and stairs.
// brickOrigin   — the staggered brick-mortar tile (column 2, 34px to the right).
//                 Used by BrickBlockRenderer and BrickShardRenderer.
//
// Previously these two were swapped; this is the corrected mapping.
inline sf::Vector2i groundOrigin(model::WorldType worldType) {
    switch (worldType) {
        case model::WorldType::Underground: return {147, 16};
        case model::WorldType::Underwater:  return {147, 100};
        case model::WorldType::Castle:      return {0, 100};
        case model::WorldType::Overworld:
        default:                            return {0, 16};
    }
}

inline sf::Vector2i brickOrigin(model::WorldType worldType) {
    // The brick tile is always 34px (2 × Pitch) to the right of the ground tile.
    const sf::Vector2i g = groundOrigin(worldType);
    return {g.x + 34, g.y};
}

}
}

#endif
