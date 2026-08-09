#include "Model/Core/GameManager.h"

namespace model {

GameManager& GameManager::instance() {
    static GameManager singleton;
    return singleton;
}

int GameManager::getScore() const {
    return score;
}

void GameManager::addScore(int points) {
    score += points;
}

int GameManager::getCoins() const {
    return coins;
}

void GameManager::addCoin(int count) {
    if (count <= 0) return;
    coins += count;
    // A loop rather than a single check: collecting several coins at once must still be able
    // to cross the threshold more than once.
    while (coins >= CoinsPerExtraLife) {
        coins -= CoinsPerExtraLife;
        addLife();
    }
}

int GameManager::getLives() const {
    return lives;
}

void GameManager::loseLife() {
    if (lives > 0) {
        --lives;
    }
}

void GameManager::addLife() {
    ++lives;
}

bool GameManager::isGameOver() const {
    return lives <= 0;
}

int GameManager::getCurrentLevel() const {
    return currentLevel;
}

void GameManager::setCurrentLevel(int level) {
    currentLevel = level;
}

void GameManager::nextLevel() {
    ++currentLevel;
}

void GameManager::startLevelTimer() {
    timeRemaining = static_cast<float>(LevelTimeUnits);
}

void GameManager::tickTimer(float deltaTime) {
    if (timeRemaining <= 0.0f) return;
    timeRemaining -= deltaTime * TimeUnitsPerSecond;
    if (timeRemaining < 0.0f) {
        timeRemaining = 0.0f;
    }
}

int GameManager::getTimeRemaining() const {
    // Truncated, not rounded: the clock should not read 400 once a tick has been spent, and
    // it must not show 1 while the level is already lost.
    return static_cast<int>(timeRemaining);
}

bool GameManager::isTimeUp() const {
    return timeRemaining <= 0.0f;
}

int GameManager::awardTimeBonus() {
    const int bonus = getTimeRemaining() * PointsPerTimeUnit;
    timeRemaining = 0.0f;  // stops the clock, and makes a repeat call a no-op
    addScore(bonus);
    return bonus;
}

void GameManager::reset() {
    score = 0;
    coins = 0;
    lives = StartingLives;
    currentLevel = FirstLevel;
    timeRemaining = 0.0f;
}

}
