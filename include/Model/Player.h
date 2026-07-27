#ifndef MODEL_PLAYER_H
#define MODEL_PLAYER_H

#include "Model/Character.h"
#include "Model/PlayerState.h"

#include <memory>

namespace model {

class Player : public Character {
public:
    Player(Vector2 position, Vector2 size);
    ~Player() override;

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void handleInput();
    void onCollision(Entity* other) override;
    void takeDamage(int amount) override;

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
    int lives;
    float damageCooldown;

private:
    void syncAnimation();
    static constexpr float DamageCooldownTime = 0.5f;
};

}

#endif
