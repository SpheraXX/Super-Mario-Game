#ifndef MODEL_MARIO_H
#define MODEL_MARIO_H

#include "Model/Player.h"

namespace model {

class Mario : public Player {
public:
    Mario(Vector2 position);

    static constexpr float WalkSpeed = 300.0f;
    static constexpr float RunSpeed = 420.0f;
    static constexpr float JumpForce = -900.0f;
};

}

#endif
