#ifndef MODEL_WORLD_WORLDTYPE_H
#define MODEL_WORLD_WORLDTYPE_H

#include <string>

namespace model {

// The playable worlds a map can declare (see TileMap header metadata: '; world=...').
// Each type resolves to a World via WorldSet, supplying the graphics theme and the
// physics every character obeys while the level is live.
enum class WorldType {
    Overworld,
    Underground,
    Underwater,
    Castle
};

inline WorldType worldTypeFromString(const std::string& value) {
    if (value == "underground") return WorldType::Underground;
    if (value == "underwater") return WorldType::Underwater;
    if (value == "castle") return WorldType::Castle;
    return WorldType::Overworld;
}

}

#endif
