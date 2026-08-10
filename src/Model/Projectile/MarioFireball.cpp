#include "Model/Projectile/MarioFireball.h"

namespace model {

MarioFireball::MarioFireball(Vector2 position, Entity* owner, int direction)
    : Projectile(position, {Width, Height}, owner) {
    velocity = {TravelSpeed * static_cast<float>(direction), 0.0f};
    setDirection(direction);
}

void MarioFireball::update(float deltaTime) {
    // The tile pass marks a landing by setting isGrounded (and zeroing vy); the next frame
    // turns that landing into a bounce. The ball never rests — it hops along until it dies.
    // A ceiling hit is left alone: vy is zeroed by the tile pass, gravity pulls it back
    // down, and the next landing bounces it again.
    if (isGrounded) {
        velocity.y = -BounceSpeed;
    }
    animationClock += deltaTime;
    Character::update(deltaTime);
}

void MarioFireball::onCollision(Entity& other, CollisionType side) {
    // Solid entity blocks (coin blocks) destroy the ball, matching tile walls. Everything
    // else follows the normal projectile rules: enemies die, items and the player pass.
    if (other.isSolid()) {
        expire();
        return;
    }
    Projectile::onCollision(other, side);
}

void MarioFireball::onTileCollision(char /* tile */, CollisionType side) {
    // Side walls destroy the ball. Bottom landings bounce (handled in update via isGrounded);
    // Top is a ceiling knock-down, which needs no extra code.
    if (side == CollisionType::Left || side == CollisionType::Right) {
        expire();
    }
}

}
