#ifndef MODEL_HAMMERBRO_H
#define MODEL_HAMMERBRO_H

#include "Model/Enemy/Enemy.h"

namespace model {

// Patrols a short beat around where the level placed it, hopping very frequently — it hops
// even while idle — and throwing a continuous supply of hammers at the player.
class HammerBro : public Enemy {
public:
    explicit HammerBro(Vector2 position);

    void updateAI(float deltaTime) override;
    std::unique_ptr<Projectile> createProjectile() override;

private:
    float patrolCentreX;  // captured at spawn: the beat is relative to the map placement
    float hopTimer;

    static constexpr float WalkSpeed = 15.0f;
    static constexpr float PatrolRange = 24.0f;   // half-width of the beat, in world units
    static constexpr float HopInterval = 1.2f;
    static constexpr float HopSpeed = -190.0f;
    static constexpr float ThrowInterval = 2.0f;
};

}

#endif
