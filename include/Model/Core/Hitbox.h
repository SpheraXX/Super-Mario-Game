#ifndef MODEL_HITBOX_H
#define MODEL_HITBOX_H

#include "Model/Core/Vector2.h"

namespace model {

enum class CollisionLayer {
    Player,
    Enemy,
    Environment,
    Trigger
};

class Hitbox {
public:
    Hitbox() : offset{0.0f, 0.0f}, width(0.0f), height(0.0f), isTrigger(false), layer(CollisionLayer::Environment) {}
    Hitbox(Vector2 offset, float width, float height, bool isTrigger, CollisionLayer layer)
        : offset(offset), width(width), height(height), isTrigger(isTrigger), layer(layer) {}

    bool intersects(const Hitbox& other, Vector2 myPos, Vector2 otherPos) const;
    Vector2 getOverlap(const Hitbox& other, Vector2 myPos, Vector2 otherPos) const;

    Vector2 offset;
    float width;
    float height;
    bool isTrigger;
    CollisionLayer layer;
};

}

#endif
