#include "Model/Level/Pipe.h"

#include "Model/Core/Hitbox.h"

namespace model {

Pipe::Pipe(Vector2 position, Vector2 size, std::size_t sourceColumn)
    : Entity(position, size), sourceColumn_(sourceColumn) {
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

bool Pipe::isSolid() const {
    return true;
}

std::size_t Pipe::getSourceColumn() const {
    return sourceColumn_;
}

}