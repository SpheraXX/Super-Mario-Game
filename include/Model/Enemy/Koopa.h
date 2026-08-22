#ifndef MODEL_KOOPA_H
#define MODEL_KOOPA_H

#include "Model/Enemy/Enemy.h"

namespace model {

enum class KoopaState {
    Walking,
    ShellIdle,
    ShellSpinning
};

class Koopa : public Enemy {
public:
    // `winged` makes this a Koopa Paratroopa (map id 2): it FLIES rather than walking —
    // gravity is switched off and it patrols vertically over the altitude the map placed it
    // at — and the first stomp knocks the wings off instead of producing a shell, dropping
    // it to an ordinary walking Koopa. It is a flag rather than a subclass because the wings
    // are one more rung on the same demotion ladder the shell states already model:
    // Paratroopa -> Koopa -> shell is a single object throughout.
    explicit Koopa(Vector2 position, bool winged = false);

    void updateAI(float deltaTime) override;
    void onStomped(Entity& player) override;
    void onCollision(Entity& other, CollisionType side) override;
    void onTileCollision(char tile, CollisionType side) override;

    // True when the Koopa is in one of its shell states (idle or spinning), used by the
    // view to pick the shell frame.
    bool isShell() const;
    // True while it still has its wings, used by the view to pick the Paratroopa frame.
    bool isWinged() const;

    KoopaState getState() const { return state; }
    void setState(KoopaState newState);

private:
    // Give up the wings: gravity comes back on and it finishes the fall as a walking Koopa.
    void loseWings();

    KoopaState state;
    float shellSpeed;
    bool winged;

    // Flight bookkeeping, only meaningful while winged. The patrol is anchored to the
    // altitude the map placed the Paratroopa at and never descends below it, so a
    // Paratroopa placed just above the floor cannot sink into it.
    float flyBaseY;
    bool flyingDown;

    static constexpr float WalkSpeed = 20.0f;
    static constexpr float SpinSpeed = 125.0f;
    // Flight: a slow horizontal cruise with a vertical patrol two tiles tall. Vertical
    // motion is driven through velocity (not by writing the position) so tile collision
    // still resolves normally and the wings cannot carry it through a ceiling.
    static constexpr float FlySpeed = 25.0f;
    static constexpr float FlyRiseSpeed = 35.0f;
    static constexpr float FlyAmplitude = 32.0f;
    // Standing Koopa is taller than one tile (16x23 art, drawn 1:1); the shell frame is
    // a square 16x16, so entering a shell state shrinks the box to match.
    static constexpr float StandHeight = 23.0f;
    static constexpr float ShellHeight = 16.0f;
};

}

#endif
