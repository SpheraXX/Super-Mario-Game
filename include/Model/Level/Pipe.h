#ifndef MODEL_LEVEL_PIPE_H
#define MODEL_LEVEL_PIPE_H

#include "Model/Entity.h"

#include <cstddef>

namespace model {

// The solid 'P' run in a map becomes one Pipe entity whose box covers the run. It blocks
// the player like a wall (standing on the top is allowed — that is where you enter), and
// PlayState polls the entity's sourceColumn to match the player against the level portal
// that owns this pipe.
class Pipe : public Entity {
public:
    Pipe(Vector2 position, Vector2 size, std::size_t sourceColumn);

    bool isSolid() const override;
    std::size_t getSourceColumn() const;

private:
    std::size_t sourceColumn_;
};

}

#endif