#ifndef MODEL_LAKITU_H
#define MODEL_LAKITU_H

#include "Model/Enemy/Enemy.h"

namespace model {

// Rides a cloud above the player, drifting to stay overhead while dropping an endless supply
// of Spiny Eggs. Floats over all terrain: no gravity, no tile collision.
//
// The only enemy that steers by the player rather than by the world.
class Lakitu : public Enemy {
public:
    explicit Lakitu(Vector2 position);

    void updateAI(float deltaTime) override;
    std::unique_ptr<Projectile> createProjectile() override;

    bool usesTileCollision() const override { return false; }

private:
    float hoverY;  // the altitude the level placed it at; it never changes height

    static constexpr float TrackSpeed = 30.0f;
    static constexpr float DeadZone = 8.0f;      // stops the hover jittering when overhead
    static constexpr float ThrowInterval = 3.0f;
    // Horizontal kick given to a dropped egg, so it lands ahead of a running player instead
    // of straight down. See DesignDoc/ENEMIES.md on why this deviates from the shipped ROM.
    static constexpr float EggLeadSpeed = 35.0f;
};

}

#endif
