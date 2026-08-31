#ifndef MODEL_CHEEPCHEEP_H
#define MODEL_CHEEPCHEEP_H

#include "Model/Enemy/Enemy.h"

namespace model {

// The underwater fish. It swims straight along its row at a constant speed and ignores
// terrain entirely — in the original it crosses rock and pipe alike, and the water levels
// are authored on that assumption, so a Cheep Cheep that collided with the seabed would
// wedge itself in the first outcrop it met.
//
// Stompable like any soft-bodied enemy, which is what makes it the water level's ordinary
// foot soldier rather than a Spiny-style hazard.
class CheepCheep : public Enemy {
public:
    explicit CheepCheep(Vector2 position, int direction = -1);

    void updateAI(float deltaTime) override;

    bool usesTileCollision() const override { return false; }

private:
    // Slower than a Goomba's walk: everything underwater moves as if it were wading, and a
    // fish at land speed is unreadable in a level where the player himself is slowed.
    static constexpr float SwimSpeed = 30.0f;
};

}

#endif
