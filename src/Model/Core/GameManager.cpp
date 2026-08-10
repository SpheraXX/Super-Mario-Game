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

void GameManager::reset() {
    score = 0;
    lives = StartingLives;
    currentLevel = FirstLevel;
}

}
