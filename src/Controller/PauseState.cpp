#include "Controller/PauseState.h"
#include "Controller/AppEngine.h"
#include "Controller/StateManager.h"
#include "Controller/OptionsState.h"
#include "Controller/MainMenuState.h"
#include "Model/SettingsManager.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"

#include <memory>
#include <iostream>

namespace controller {

PauseState::PauseState() 
    : menuContainer(view::ui::UIContainer::Layout::Vertical, 10.f) {
    
    // Dim the screen underneath
    overlay.setSize({static_cast<float>(AppEngine::screenWidth()),
                     static_cast<float>(AppEngine::ScreenHeight)});
    overlay.setFillColor(view::ui::theme::ColorOverlay);

    const sf::Font& font = view::AssetManager::instance().getUiFont();

    titleLabel = view::ui::UILabel(font, "PAUSED", 16);
    // Center title horizontally
    float screenW = static_cast<float>(AppEngine::screenWidth());
    titleLabel.setPosition((screenW - 96.f) / 2.f, 40.f);

    menuContainer.setPosition((screenW - 120.f) / 2.f, 90.f);

    auto btnResume = std::make_unique<view::ui::UIButton>(
        font, "RESUME", 8, sf::Vector2f(0.f, 0.f), sf::Vector2f(120.f, 25.f));
    btnResume->setOnClick([this]() {
        if (manager) manager->popState();
    });

    auto btnOptions = std::make_unique<view::ui::UIButton>(
        font, "OPTIONS", 8, sf::Vector2f(0.f, 0.f), sf::Vector2f(120.f, 25.f));
    btnOptions->setOnClick([this]() {
        if (manager) manager->pushState(std::make_unique<OptionsState>());
    });

    auto btnMainMenu = std::make_unique<view::ui::UIButton>(
        font, "MAIN MENU", 8, sf::Vector2f(0.f, 0.f), sf::Vector2f(120.f, 25.f));
    btnMainMenu->setOnClick([this]() {
        if (manager) {
            // TODO: [SAVE] save progress here
            manager->clear();
            manager->pushState(std::make_unique<MainMenuState>());
        }
    });

    menuContainer.add(std::move(btnResume));
    menuContainer.add(std::move(btnOptions));
    menuContainer.add(std::move(btnMainMenu));
}

void PauseState::onDisplayModeChanged() {
    float screenW = static_cast<float>(AppEngine::screenWidth());
    float screenH = static_cast<float>(AppEngine::ScreenHeight);
    
    overlay.setSize({screenW, screenH});
    titleLabel.setPosition((screenW - 96.f) / 2.f, 40.f);
    menuContainer.setPosition((screenW - 120.f) / 2.f, 90.f);
    menuContainer.relayout();
}

void PauseState::onResume() {
    onDisplayModeChanged();
}

void PauseState::update(float dt) {
    menuContainer.update(dt);
}

void PauseState::render(sf::RenderTarget& target) {
    target.draw(overlay);
    titleLabel.render(target);
    menuContainer.render(target);
}

void PauseState::handleEvent(const sf::Event& event) {
    // If user presses the Pause key again, resume the game
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (static_cast<int>(key->code) == model::SettingsManager::instance().get().keyPause) {
            if (manager) manager->popState();
            return;
        }
    }
    menuContainer.handleEvent(event);
}

} // namespace controller
