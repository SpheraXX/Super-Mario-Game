#ifndef MODEL_GOOMBA_H
#define MODEL_GOOMBA_H

#include "Model/Enemy.h"

namespace model {

class Goomba : public Enemy {
public:
    Goomba(Vector2 position);

    void updateAI(float deltaTime) override;
    void onStomped(Entity& player) override;
    void onTileCollision(char tile, CollisionType side) override;

private:
    float squishTimer;
    static constexpr float WalkSpeed = 50.0f;
};

}

#endif
