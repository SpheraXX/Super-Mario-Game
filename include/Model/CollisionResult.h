#ifndef MODEL_COLLISIONRESULT_H
#define MODEL_COLLISIONRESULT_H

namespace model {

enum class CollisionType {
    None,
    Top,
    Bottom,
    Left,
    Right
};

struct CollisionResult {
    bool collided = false;
    CollisionType type = CollisionType::None;
    class Entity* other = nullptr;
};

}

#endif
