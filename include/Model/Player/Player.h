#ifndef MODEL_PLAYER_H
#define MODEL_PLAYER_H

#include "Model/Character.h"
#include "Model/Player/PlayerState.h"

#include <memory>

namespace model {

class Player : public Character {
public:
    Player(Vector2 position, Vector2 size);
    ~Player() override;

    void update(float deltaTime) override;

    void handleInput() override;
    void onCollision(Entity* other) override;
    void takeDamage(int amount) override;

    using Character::die;
    // Full death: lose a life and play the pop/fall death animation. bounce=false is
    // used for pit falls (the body just keeps dropping).
    void die(bool bounce);

    void setState(std::unique_ptr<PlayerState> newState);
    PlayerState& getState();
    const char* getStateName() const;
    float getRemainingTime() const;

    void becomeSuper();
    void becomeFire();
    void becomeStar();

    // Convenience forwards to GameManager, which is the single owner of these counters:
    // the player object does not survive a death, and the totals must. Coins are not here
    // at all — coin sources talk to GameManager directly, so there is one rule and one path.
    void addScore(int points);
    void addLife();

    int getScore() const;
    int getLives() const;

protected:
    std::unique_ptr<PlayerState> state;
    float damageCooldown;
    // Press-edge jump tracking: jumpHeld remembers the raw button state from the
    // previous frame so a held key cannot re-trigger a jump on landing; playerInitiatedJump
    // marks the current ascent as started by the player (so releasing it cuts the jump,
    // but a stomp bounce is never cut).
    bool jumpHeld = false;
    bool playerInitiatedJump = false;

private:
    void syncAnimation();
    static constexpr float DamageCooldownTime = 0.5f;
};

}

#endif
