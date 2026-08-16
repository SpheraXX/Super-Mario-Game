#include "Model/Level/FirebarBall.h"

#include "Model/Core/Hitbox.h"

#include <cmath>

namespace model {

namespace {
constexpr float BallSize = 8.0f;
}

FirebarBall::FirebarBall(Vector2 pivot, float radius, float angularSpeed, float phase)
    : Projectile(pivot, {BallSize, BallSize}, nullptr),
      pivot(pivot),
      radius(radius),
      angularSpeed(angularSpeed),
      angle(phase) {
    // No owner, so Projectile::firedByPlayer() is false and the ball targets the player —
    // the same wiring Bowser's fire uses.
    setGravityScale(0.0f);
    update(0.0f);  // place it on its arc before the first frame draws
}

void FirebarBall::update(float deltaTime) {
    angle += angularSpeed * deltaTime;

    // Position is a pure function of the angle rather than an integrated velocity: the arm
    // can never drift off its circle, and a ball added mid-level lands exactly in step with
    // the rest of its bar. Character::update is deliberately NOT called — this thing has no
    // gravity, no death fall and no velocity to integrate.
    const Vector2 centre{pivot.x + std::cos(angle) * radius,
                         pivot.y + std::sin(angle) * radius};
    setPosition({centre.x - BallSize * 0.5f, centre.y - BallSize * 0.5f});
}

void FirebarBall::onCollision(Entity& other, CollisionType /* side */) {
    if (!isActive || !isTarget(other)) return;
    if (auto* character = dynamic_cast<Character*>(&other)) {
        character->takeDamage(damageValue);
    }
    // No expire(): unlike a thrown hammer, the bar is still there afterwards.
}

}
