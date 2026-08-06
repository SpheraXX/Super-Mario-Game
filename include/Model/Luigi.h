#ifndef MODEL_LUIGI_H
#define MODEL_LUIGI_H

#include "Model/Player.h"

namespace model {

class Luigi : public Player {
public:
    Luigi(Vector2 position);

    static constexpr float WalkSpeed = 260.0f;
    static constexpr float RunSpeed = 380.0f;
    static constexpr float JumpForce = -1000.0f;
};

}

#endif
