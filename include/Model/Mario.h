#ifndef MODEL_MARIO_H
#define MODEL_MARIO_H

#include "Model/Player.h"

namespace model {

class Mario : public Player {
public:
    Mario(Vector2 position);

    static constexpr float WalkSpeed = 180.0f;
    static constexpr float RunSpeed = 400.0f;
    static constexpr float JumpForce = -450.0f;
};

}

#endif
