#ifndef MODEL_LUIGI_H
#define MODEL_LUIGI_H

#include "Model/Player.h"

namespace model {

class Luigi : public Player {
public:
    Luigi(Vector2 position);

    static constexpr float WalkSpeed = 160.0f;
    static constexpr float RunSpeed = 350.0f;
    static constexpr float JumpForce = -540.0f;
};

}

#endif
