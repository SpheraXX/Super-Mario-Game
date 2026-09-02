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
    //
    // stompedBy() is the entry point the collision pass calls; onStomped() is what
    // subclasses override. The split exists because the one-stomp-per-contact lockout has
    // to hold for every enemy, and Koopa (the one subclass with a multi-step reaction)
    // deliberately does not chain to Enemy::onStomped, so the lockout cannot live there.
    void stompedBy(Entity& player);
    virtual void onStomped(Entity& player);

    // False while the player is still passing through this enemy just after stomping it,
    // and for a squished body waiting to despawn. The collision pass skips the pair
    // entirely in that window: no repeat stomp, and no contact damage either.
    //
    // This matters because nothing pushes the two apart. The player keeps his momentum
    // through a stomp and falls on past the enemy, overlapping it for several frames; every
    // one of those frames would otherwise read as a fresh stomp (awarding the score again,
    // or running a Koopa through its whole shell ladder in a fraction of a second), and the
    // frames where he has sunk far enough to read as a SIDE hit would damage him — so a
    // successful stomp would hurt the player.
    bool acceptsPlayerContact() const;

    // Keep the pair inert for another beat. The collision pass calls this on every frame
    // the player is still inside an enemy he has already stomped, so the lockout does not
    // expire underneath him: with no bounce he lands in the same spot the enemy occupies
    // (a stomped Koopa's shell sits exactly where he comes to rest), and a lockout on a
    // plain timer would run out while he stands there and then damage him for a stomp he
    // had already won. Contact only re-arms once he has actually moved clear.
    void holdStompLockout();
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

    // How long the squish sprite is shown before the body despawns. Goomba overrides to
    // a shorter window (0.5s), Koopa re-uses this default (1s) for the shell collapse.
    static constexpr float DefaultSquishDuration = 1.0f;

    // Ticks the attack cooldown and fires createProjectile() into the world when it elapses.
    // Inert unless a subclass sets attackCooldown.
    void updateAttack(float deltaTime);

    // Where the player is right now, or nullptr. For the few enemies that aim rather than
    // patrol blindly (Hammer Bro, Lakitu).
    const Entity* findPlayer() const;

    int damageValue;
    bool isStomped;
    float despawnTimer;

    // Counts down after a stomp; see acceptsPlayerContact(). Long enough for the player to
    // fall clear of a one-tile enemy at stomp speeds, short enough that it never blocks a
    // deliberate second hit (re-stomping a Koopa shell needs a fresh jump, which takes far
    // longer than this).
    static constexpr float StompLockoutTime = 0.2f;
    float stompLockout = 0.0f;

    float attackCooldown = 0.0f;  // seconds between attacks; 0 disables attacking entirely
    float attackTimer = 0.0f;     // counts down to the next attack
};

}

#endif
