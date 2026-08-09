#ifndef MODEL_GAMEMANAGER_H
#define MODEL_GAMEMANAGER_H

namespace model {

// Singleton holding the global, cross-state game progress (score, coins, lives, level, and
// the level timer). Accessed everywhere through GameManager::instance().
//
// This is deliberately the only home for these counters. The player entity is destroyed and
// rebuilt on every death (see PlayState::resetLevel), so anything stored on the player would
// be wiped each time a life is lost — score and coins have to outlive it.
class GameManager {
public:
    static GameManager& instance();

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    int getScore() const;
    void addScore(int points);

    int getCoins() const;
    // Every CoinsPerExtraLife coins collected grants a life and the tally rolls back to zero.
    void addCoin(int count = 1);

    int getLives() const;
    void loseLife();
    void addLife();
    bool isGameOver() const;

    int getCurrentLevel() const;
    void setCurrentLevel(int level);
    void nextLevel();

    // --- Level timer -------------------------------------------------------------------
    // A per-level allowance that counts down while the level is being played. It is a bonus
    // budget: whatever is left when the level is finished converts into score. Running it
    // dry is fatal.

    // Refill the clock. Called at the start of a level and on every retry after a death.
    void startLevelTimer();
    // Advance the clock. Callers stop calling this while the player is dying or the level is
    // over, which is what freezes the display at those moments.
    void tickTimer(float deltaTime);
    // Whole units remaining, which is what the HUD shows.
    int getTimeRemaining() const;
    bool isTimeUp() const;
    // Convert the unspent clock into score and stop it. Returns the points awarded so the
    // caller can show them. Safe to call twice: the second call awards nothing.
    int awardTimeBonus();

    // Restore starting values for a brand new game.
    void reset();

    static constexpr int StartingLives = 3;
    static constexpr int FirstLevel = 1;
    // Coins needed for a 1-up. The original uses 100; this clone is shorter, so 50.
    static constexpr int CoinsPerExtraLife = 50;
    // Timer units granted per level, and how fast they tick. The original's clock runs
    // faster than real time — 400 units at 2.5/second is a little under three minutes.
    static constexpr int LevelTimeUnits = 400;
    static constexpr float TimeUnitsPerSecond = 2.5f;
    static constexpr int PointsPerTimeUnit = 50;

private:
    GameManager() = default;

    int score = 0;
    int coins = 0;
    int lives = StartingLives;
    int currentLevel = FirstLevel;
    // Fractional so the countdown is smooth; only whole units are ever displayed.
    float timeRemaining = 0.0f;
};

}

#endif
