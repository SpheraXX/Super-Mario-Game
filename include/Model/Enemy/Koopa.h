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
    // `winged` makes this a Koopa Paratroopa (map id 2): it hops as it walks, and the first
    // stomp knocks the wings off instead of producing a shell. It is a flag rather than a
    // subclass because the wings are one more rung on the same demotion ladder the shell
    // states already model — Paratroopa -> Koopa -> shell is a single object throughout.
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

private:
    KoopaState state;
    float shellSpeed;
    bool winged;
    static constexpr float WalkSpeed = 40.0f;
    static constexpr float SpinSpeed = 250.0f;
    static constexpr float HopSpeed = -300.0f;
    // World height of the shell frame (16px art at 2x), against 64 standing.
    static constexpr float ShellHeight = 32.0f;
};

}

#endif
