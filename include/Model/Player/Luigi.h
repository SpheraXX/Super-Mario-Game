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

    // The view picks its spritesheet rows off this rather than dynamic_casting the
    // concrete type; without the override Luigi would render with Mario's palette.
    bool isLuigi() const override { return true; }

    static constexpr float WalkSpeed = 80.0f;
    static constexpr float RunSpeed = 175.0f;
    static constexpr float MaxJumpSpeed = 300.0f;
    static constexpr float JumpAccel = 1700.0f;
};

}

#endif
