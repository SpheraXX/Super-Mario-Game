#ifndef MODEL_MARIO_H
#define MODEL_MARIO_H

#include "Model/Player/Player.h"

namespace model {

class Mario : public Player {
public:
    Mario(Vector2 position);

    float getWalkSpeed() const override;
    float getRunSpeed() const override;
    float getMaxJumpSpeed() const override;
    float getJumpAccel() const override;
    float getStompBounceRatio() const override;
    float getStompBounceConstant() const override;

    static constexpr float WalkSpeed = 180.0f;
    // Test tuning: doubled from the normal 90/180 pair so map traversal is faster.
    static constexpr float RunSpeed = 360.0f;
    // Safety ceiling only: with the hold window and accel below, the boost tops out near
    // 495px/s, so a full-length hold never reaches this cap.
    static constexpr float MaxJumpSpeed = 340.0f;
    // Jump accel. With Gravity 1600 and the 0.16s hold, the net boost rate is
    // (JumpAccel - Gravity) ≈ 1500px/s^2: a full hold ends the ascent boost at ~470px/s,
    // and the total rise (impulse + boost + coast) comes out ≈137px — BARELY over the
    // 128px of a 4-tile wall, clearly short of the 160px of a 5-tile one. The weak graze
    // at the top of that arc is below the bump-speed gate in CollisionManager, so it
    // cannot open a block 5 tiles above the feet. Tap jumps rise much less.
    static constexpr float JumpAccel = 1550.0f;

    // Stomp-bounce rebound (see Player::getStompBounceRatio): Mario keeps the stock feel —
    // 0.85 of the fall speed minus 30 — so a normal ~350px/s drop relaunches him at ~267px/s.
    static constexpr float StompBounceRatio = 0.85f;
    static constexpr float StompBounceConstant = 30.0f;
};

}

#endif
