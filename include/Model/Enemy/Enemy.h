#ifndef MODEL_ENEMY_H
#define MODEL_ENEMY_H

#include "Model/Character.h"

#include <memory>

namespace model {

class Projectile;

class Enemy : public Character {
public:
    Enemy(Vector2 position, Vector2 size);
    virtual ~Enemy() = default;

    void update(float deltaTime) override;
    virtual void updateAI(float deltaTime) = 0;

    // Stomp and damage hooks are Enemy semantics: CollisionManager dispatches them via
    // the collision layers, and only Enemy subclasses react. These are declared first
    // here (not on Entity), so there is nothing to override — a pipe is never stomped.
    virtual void onStomped(Entity& player);
    // Knocked out by another enemy (e.g. a spinning shell): pop up and fall away.
    virtual void onHit(Entity& source);
    int getDamageValue() const override;

    // Factory Method. Enemies that attack override this to say *what* they throw; the
    // cooldown and the handoff to the world are handled once, in updateAttack(). Returning
    // nullptr (the default) simply means this enemy does not attack.
    // Defined out of line: the default body would otherwise need Projectile complete here,
    // and every enemy header would have to pull it in.
    virtual std::unique_ptr<Projectile> createProjectile();

    // Points for defeating this enemy. Overridden by the ones worth more than a foot soldier.
    virtual int getScoreValue() const { return DefaultScoreValue; }

    // True while the enemy shows its squished sprite (stomped but not gone yet).
    bool isSquished() const;

protected:
    // Credit this enemy's value to the score. Called from wherever a subclass decides it has
    // actually been defeated — which is not every stomp, since kicking a Koopa shell around
    // is not a kill.
    void awardScore() const;

    static constexpr int DefaultScoreValue = 100;

    // Ticks the attack cooldown and fires createProjectile() into the world when it elapses.
    // Inert unless a subclass sets attackCooldown.
    void updateAttack(float deltaTime);

    // Where the player is right now, or nullptr. For the few enemies that aim rather than
    // patrol blindly (Hammer Bro, Lakitu).
    const Entity* findPlayer() const;

    int damageValue;
    bool isStomped;
    float despawnTimer;

    float attackCooldown = 0.0f;  // seconds between attacks; 0 disables attacking entirely
    float attackTimer = 0.0f;     // counts down to the next attack
};

}

#endif
