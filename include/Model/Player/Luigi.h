#ifndef MODEL_LUIGI_H
#define MODEL_LUIGI_H

#include "Model/Player/Player.h"

namespace model {

class Luigi : public Player {
public:
    Luigi(Vector2 position);

    float getWalkSpeed() const override;
    float getRunSpeed() const override;
    float getJumpForce() const override;
    bool isLuigi() const override;

    static constexpr float WalkSpeed = 160.0f;
    static constexpr float RunSpeed = 350.0f;
    static constexpr float JumpForce = -720.0f;
};

}

#endif
