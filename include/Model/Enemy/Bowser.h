#ifndef MODEL_BOWSER_H
#define MODEL_BOWSER_H

#include "Model/Enemy/Enemy.h"

namespace model {

// Paces a short stretch, jumps periodically, and breathes fire at the player. Not stompable:
// landing on him damages the player. Takes five hits to defeat, matching the original's
// five-fireball kill; the axe and the collapsing bridge are level scripting, not enemy logic.
class Bowser : public Enemy {
public:
    explicit Bowser(Vector2 position);

    void updateAI(float deltaTime) override;
    std::unique_ptr<Projectile> createProjectile() override;
    void onHit(Entity& source) override;

    bool isStompable() const override { return false; }

private:
    float patrolCentreX;
    float jumpTimer;

    static constexpr float WalkSpeed = 30.0f;
    static constexpr float PatrolRange = 64.0f;
    static constexpr float JumpInterval = 3.0f;
    static constexpr float JumpSpeed = -420.0f;
    static constexpr float FireInterval = 2.5f;
    static constexpr int MaxHealth = 5;
    // Must track Fireball's world width, used to place the breath clear of his left side.
    static constexpr float FireballWidth = 48.0f;
};

}

#endif
