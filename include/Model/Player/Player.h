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

    void addScore(int points);
    void addCoin();
    void addLife();

    int getScore() const;
    int getCoins() const;
    int getLives() const;

protected:
    std::unique_ptr<PlayerState> state;
    int score;
    int coins;
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
