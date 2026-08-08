#include "Model/Level/Castle.h"

#include "Model/Core/Hitbox.h"

namespace model {

Castle::Castle(Vector2 position, Vector2 size)
    : Entity(position, size) {
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, false, CollisionLayer::Environment);
}

bool Castle::isSolid() const {
    return true;
}

bool Castle::isLandable() const {
    return false;
}

}
