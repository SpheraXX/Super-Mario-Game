#ifndef VIEW_BLOCK_BLOCKATLAS_H
#define VIEW_BLOCK_BLOCKATLAS_H

#include "Model/World/WorldType.h"

#include <SFML/System/Vector2.hpp>

namespace view {
namespace atlas {

// Where each landscape's block quadrant begins in the shared sheet (super_mario_asset.png)
// — the brick IS each quadrant's origin, and the solid block / stair tile are offset from
// it (see TileMapRenderer::atlasFor). Kept in exactly one place so the brick, its renderer
// and the shards it breaks into can never drift out of step with the terrain tileset.
inline sf::Vector2i brickOrigin(model::WorldType worldType) {
    switch (worldType) {
        case model::WorldType::Underground: return {147, 16};
        case model::WorldType::Underwater:  return {147, 100};
        case model::WorldType::Castle:      return {0, 100};
        case model::WorldType::Overworld:
        default:                            return {0, 16};
    }
}

}
}

#endif
