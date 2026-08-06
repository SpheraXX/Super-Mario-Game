#ifndef MODEL_LUIGI_H
#define MODEL_LUIGI_H

#include "Model/Player/Player.h"

namespace model {

class Luigi : public Player {
public:
    Luigi(Vector2 position);

    float getWalkSpeed() const override;
    float getRunSpeed() const override;
    float getMaxJumpSpeed() const override;
    float getJumpAccel() const override;

    static constexpr float WalkSpeed = 160.0f;
    static constexpr float RunSpeed = 350.0f;
    static constexpr float MaxJumpSpeed = 600.0f;
    static constexpr float JumpAccel = 3400.0f;
};

}

#endif
