#ifndef CONTROLLER_LEVELCOMPLETION_H
#define CONTROLLER_LEVELCOMPLETION_H

#include "Model/Map/TileMap.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace model {
class Entity;
class FlagPole;
}

namespace controller {

// The goal zone appended to every map: 16 padded columns holding the flagpole
// (6 tiles into the zone) and the painted castle (11 tiles in, 5 tiles wide).
// Paints the castle into the grid and spawns the flagpole; owns the pole pointer
// and the door coordinate the clear play walks to.
class LevelCompletion {
public:
    // Forget the flagpole: called at the start of every level (re)build.
    void clear();

    // Paint the goal castle into the grid and spawn the flagpole (guard: with a failed
    // load the map has no padding and there is nothing to spawn). The paint is
    // deterministic, so re-running resetLevel (enter/death) is idempotent.
    void build(model::TileMap& map,
               std::vector<std::unique_ptr<model::Entity>>& entities);

    // The flagpole was touched (the owner starts the clear play when the player is
    // alive).
    bool isTouched() const;

    model::FlagPole* flagPole() const;

    // X position of the painted castle's door — the walk target of the clear play.
    float castleDoorX(const model::TileMap& map) const;

    // Size of the completion zone appended to every map; the scene pads the final
    // area's grid with this many columns before calling build.
    static constexpr std::size_t LevelPaddingTiles = 16;

private:
    static constexpr std::size_t PoleOffsetTiles = 6;
    static constexpr std::size_t CastleOffsetTiles = 11;

    model::FlagPole* flagPolePtr = nullptr;  // non-owning: spawned by build
};

}

#endif