#ifndef MODEL_PIRANHAPLANT_H
#define MODEL_PIRANHAPLANT_H

#include "Model/Enemy/Enemy.h"

namespace model {

// Lives inside a pipe, rising out of the mouth and sinking back on a fixed cycle.
//
// It is the one enemy that never moves under physics: it has no gravity, and it deliberately
// skips tile collision, because the pipe it lives in is solid and would otherwise eject it
// the moment the tile pass ran. Its position is driven straight off a timer against the pipe
// mouth captured when the level placed it.
//
// Deviation from the original, by design decision: it emerges purely on the clock and does
// not check whether the player is standing on the pipe. The original suppresses emergence
// when Mario is close, which is what stops a pipe becoming an unavoidable hit.
class PiranhaPlant : public Enemy {
public:
    // `pipeMouthTopLeft` is the world position of the top-left cell of the pipe it belongs
    // to. The plant centres itself across the pipe's two-cell width from there.
    explicit PiranhaPlant(Vector2 pipeMouthTopLeft);

    void updateAI(float deltaTime) override;

    // Solid pipe, no physics, and spikes on every side.
    bool usesTileCollision() const override { return false; }
    bool isStompable() const override { return false; }
    // Drawn before the tile map so the pipe hides it while it is down.
    bool drawsBehindTerrain() const override { return true; }

    // World size: the 16x23 source art, drawn 1:1.
    static constexpr float Width = 16.0f;
    static constexpr float Height = 23.0f;

private:
    enum class Phase { Hidden, Rising, Extended, Retracting };

    void advancePhase();
    // 0 while fully inside the pipe, 1 while fully out.
    float extension() const;

    Phase phase;
    float phaseTimer;
    float mouthY;  // world y of the pipe's mouth line; the plant never leaves this column

    static constexpr float HiddenTime = 2.0f;
    static constexpr float RiseTime = 1.0f;
    static constexpr float ExtendedTime = 2.0f;
    static constexpr float RetractTime = 1.0f;
};

}

#endif
