#ifndef MODEL_WORLD_WORLDSET_H
#define MODEL_WORLD_WORLDSET_H

#include "Model/World/WorldTheme.h"

namespace model {

// Registry (Factory) of every world the game knows. PlayState resolves the current
// map's WorldType here, then hands the resulting World to the characters via
// Character::setWorld and to the renderer as the theme.
class WorldSet {
public:
    // The world descriptor for a type. Falls back to the Overworld for unknown types
    // so a map with a typo'ed header still plays with sane defaults.
    static const WorldTheme& forType(WorldType type);
};

}

#endif
