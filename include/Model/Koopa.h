#ifndef MODEL_KOOPA_H
#define MODEL_KOOPA_H

#include "Model/Enemy.h"

namespace model {

enum class KoopaState {
    Walking,
    ShellIdle,
    ShellSpinning
};

class Koopa : public Enemy {
public:
    Koopa(Vector2 position);

    void updateAI(float deltaTime) override;
    void onStomped(Entity& player) override;
    void onCollision(Entity& other, CollisionType side) override;
    void onTileCollision(char tile, CollisionType side) override;

    // True when the Koopa is in one of its shell states (idle or spinning), used by the
    // view to pick the shell frame.
    bool isShell() const;

private:
    KoopaState state;
    float shellSpeed;
    static constexpr float WalkSpeed = 40.0f;
    static constexpr float SpinSpeed = 250.0f;
};

}

#endif
