#ifndef MODEL_LEVEL_PIPE_H
#define MODEL_LEVEL_PIPE_H

#include "Model/Entity.h"

#include <cstddef>

namespace model {

// The solid 'P' run in a map becomes one Pipe entity whose box covers the run. It blocks
// the player like a wall (standing on the top is allowed — that is where you enter), and
// PlayState polls the entity's sourceColumn to match the player against the level portal
// that owns this pipe.
//
// A Vertical pipe is entered by standing on its cap and holding Down; the entity exists
// only when a '; pipe=' token binds one, since the standing pipe is already solid terrain
// (TileMap::isSolidTile) regardless. A Horizontal pipe (TileMap::HorizontalPipeSymbol) is
// entered by simply touching it — its mouth is too short to spare the alignment margins a
// cap can, so there is no held direction to press — and its entity is always spawned: the
// 4x2 footprint has no per-cell terrain equivalent, so the entity is its only source of
// collision, portal-bound or not. See PortalSystem::findEntryPortal for the two checks.
class Pipe : public Entity {
public:
    enum class Orientation { Vertical, Horizontal };

    Pipe(Vector2 position, Vector2 size, std::size_t sourceColumn,
         Orientation orientation = Orientation::Vertical);

    bool isSolid() const override;
    std::size_t getSourceColumn() const;
    Orientation getOrientation() const;

private:
    std::size_t sourceColumn_;
    Orientation orientation_;
};

}

#endif