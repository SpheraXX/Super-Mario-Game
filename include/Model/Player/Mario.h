#ifndef MODEL_MARIO_H
#define MODEL_MARIO_H

#include "Model/Player/Player.h"

namespace model {

class Mario : public Player {
public:
    Mario(Vector2 position);

    float getWalkSpeed() const override;
    float getRunSpeed() const override;
    float getJumpForce() const override;

    static constexpr float WalkSpeed = 180.0f;
    static constexpr float RunSpeed = 400.0f;
    static constexpr float JumpForce = -600.0f;
};

}

#endif
