#ifndef MODEL_GOOMBA_H
#define MODEL_GOOMBA_H

#include "Model/Enemy/Enemy.h"

namespace model {

class Goomba : public Enemy {
public:
    Goomba(Vector2 position);

    void updateAI(float deltaTime) override;
    void onStomped(Entity& player) override;
    void onTileCollision(char tile, CollisionType side) override;

private:
    static constexpr float WalkSpeed = 25.0f;
    // Goomba's squish is brief: the flattened sprite is only shown for half a second.
    static constexpr float SquishDuration = 0.5f;
};

}

#endif
