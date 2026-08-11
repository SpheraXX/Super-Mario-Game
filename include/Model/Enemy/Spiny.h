#ifndef MODEL_SPINY_H
#define MODEL_SPINY_H

#include "Model/Enemy/Enemy.h"

namespace model {

// Walks like a Goomba, but its shell is covered in spikes: landing on one damages the player
// instead of squashing it. Killed by a spinning shell (and by fireballs, once the player has
// them). Normally arrives by hatching from a Lakitu's Spiny Egg, though a level may also
// place one directly.
class Spiny : public Enemy {
public:
    explicit Spiny(Vector2 position);

    void updateAI(float deltaTime) override;
    void onTileCollision(char tile, CollisionType side) override;

    // The whole point of a Spiny.
    bool isStompable() const override { return false; }

private:
    static constexpr float WalkSpeed = 50.0f;
};

}

#endif
