#ifndef CONTROLLER_LEVELGEOMETRY_H
#define CONTROLLER_LEVELGEOMETRY_H

#include "Model/Map/TileMap.h"

#include <cstddef>

namespace controller {
namespace geometry {

// Which symbols count as ground for the top-face scan (solid base only: ground, the
// stair block, castle, brick and the underwater blocks).
inline bool isGroundSymbol(char symbol) {
    return symbol == 'G' || symbol == model::TileMap::StairSymbol
        || symbol == 'C' || symbol == 'B' || symbol == '#';
}

// Top face of the ground stack at the given column: scan up from the bottom row and
// return the top edge of the contiguous solid base. Falls back to a standard height.
inline float groundTopAt(const model::TileMap& map, std::size_t column) {
    const std::size_t rows = map.getRows();
    if (column < map.getColumns()) {
        std::size_t solid = 0;
        while (solid < rows && isGroundSymbol(map.getTile(solid, column))) {
            ++solid;
        }
        if (solid > 0) {
            return static_cast<float>((rows - 1 - (solid - 1)) * model::TileMap::TileHeight);
        }
    }
    return static_cast<float>((rows - 2) * model::TileMap::TileHeight);
}

}
}

#endif
