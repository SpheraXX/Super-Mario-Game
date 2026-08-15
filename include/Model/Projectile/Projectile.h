#ifndef MODEL_PROJECTILE_H
#define MODEL_PROJECTILE_H

#include "Model/Character.h"

namespace model {

// Anything an entity throws, drops, or breathes: hammers, Spiny Eggs, fireballs.
//
// Derives from Character to reuse gravity and velocity integration. It inherits a little it
// does not need (health, the death animation) — that is a deliberate trade to keep the class
// count down. Both unused parts are neutralised here: a projectile is never stompable, and
// it leaves play by simply going inactive rather than playing out a death fall.
class Projectile : public Character {
public:
    Projectile(Vector2 position, Vector2 size, Entity* owner);

    void onCollision(Entity& other, CollisionType side) override;

    // A projectile is never a platform. Without this, landing on a hammer would bounce the
    // player like a stomped Goomba.
    bool isStompable() const override { return false; }

    int getDamageValue() const override { return damageValue; }

    // Leave play immediately. Projectiles have no death animation to run.
    void expire();

protected:
    // True if `other` is on the side this projectile was fired against: an owner-fired-by-
    // the-player hurts enemies, anything else hurts the player. One class covers Bowser's
    // fire and Mario's fireball because the owner decides, not the type.
    bool isTarget(const Entity& other) const;
    bool firedByPlayer() const;

    Entity* owner;  // non-owning; may dangle only if the owner outlives the level, which it cannot
    int damageValue;
};

}

#endif
