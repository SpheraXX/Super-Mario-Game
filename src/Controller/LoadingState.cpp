#include "Controller/LoadingState.h"
#include "Controller/AppEngine.h"
#include "Controller/StateManager.h"
#include "View/AssetManager.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <cmath>

namespace controller {

LoadingState::LoadingState(std::function<void()> workCallback, std::unique_ptr<GameState> nextState)
    : m_workCallback(std::move(workCallback)), 
      m_nextState(std::move(nextState)),
      m_spinnerSprite(view::AssetManager::instance().getTexture("assets/characters.png")) {
    // Crop a part of mario (e.g. 16x16 at top left)
    m_spinnerSprite.setTextureRect(sf::IntRect({0, 0}, {16, 16}));
    m_spinnerSprite.setOrigin({8.f, 8.f});
    m_spinnerSprite.setScale({4.f, 4.f});

    m_background.setFillColor(sf::Color(20, 20, 20)); // Dark background
}

void LoadingState::onEnter() {
    onDisplayModeChanged();
    m_firstFrameDone = false;
}

void LoadingState::onDisplayModeChanged() {
    float w = static_cast<float>(AppEngine::screenWidth());
    float h = static_cast<float>(AppEngine::ScreenHeight);
    
    m_background.setSize({w, h});
    m_spinnerSprite.setPosition({w / 2.f, h / 2.f});
}

void LoadingState::handleEvent(const sf::Event&) {
    // Ignore inputs while loading
}

void LoadingState::update(float) {
    // Update spinner rotation (animation state mutation belongs here, not in render)
    m_rotation += 5.f;
    m_spinnerSprite.setRotation(sf::degrees(m_rotation));

    if (!m_firstFrameDone) {
        m_firstFrameDone = true;
        return;
    }

    if (m_workCallback) {
        m_workCallback();
        m_workCallback = nullptr;
    }

    if (m_nextState) {
        manager->replaceState(std::move(m_nextState));
    }
}

void LoadingState::render(sf::RenderTarget& target) {
    target.draw(m_background);
    target.draw(m_spinnerSprite);
}

} // namespace controller
