#include "Controller/PauseState.h"
#include "Controller/AppEngine.h"
#include "Controller/StateManager.h"
#include "Controller/OptionsState.h"
#include "Controller/MainMenuState.h"
#include "Controller/WarningPopupState.h"
#include "Model/Core/LogManager.h"
#include "Model/SettingsManager.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"

#include <memory>
#include <iostream>

namespace controller {

PauseState::PauseState(std::function<void()> onSave, std::function<void()> onRestart) 
    : onSaveCallback(std::move(onSave)), onRestartCallback(std::move(onRestart)),
      menuContainer(view::ui::UIContainer::Layout::Vertical, view::ui::layout::MenuButtonGap) {
    
    // Dim the screen underneath
    overlay.setSize({static_cast<float>(AppEngine::screenWidth()),
                     static_cast<float>(AppEngine::ScreenHeight)});
    overlay.setFillColor(view::ui::theme::ColorOverlay);

    const sf::Font& font = view::AssetManager::instance().getUiFont();

    titleLabel = view::ui::UILabel(font, "PAUSED", view::ui::layout::TitleFontSize);
    // Center title horizontally
    float screenW = static_cast<float>(AppEngine::screenWidth());
    titleLabel.setPosition((screenW - 96.f) / 2.f, 35.f);

    menuContainer.setPosition((screenW - view::ui::layout::MenuButtonWidth) / 2.f, 80.f);

    auto btnResume = std::make_unique<view::ui::UIButton>(
        font, "RESUME", view::ui::layout::ButtonFontSize, sf::Vector2f(0.f, 0.f), sf::Vector2f(view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight));
    btnResume->setOnClick([this]() {
        model::LogManager::instance().info("Resume");
        if (manager) manager->popState();
    });

    auto btnRestart = std::make_unique<view::ui::UIButton>(
        font, "RESTART", view::ui::layout::ButtonFontSize, sf::Vector2f(0.f, 0.f), sf::Vector2f(view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight));
    btnRestart->setOnClick([this]() {
        if (onRestartCallback) {
            onRestartCallback();
        }
        if (manager) manager->popState();
    });

    auto btnOptions = std::make_unique<view::ui::UIButton>(
        font, "OPTIONS", view::ui::layout::ButtonFontSize, sf::Vector2f(0.f, 0.f), sf::Vector2f(view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight));
    btnOptions->setOnClick([this]() {
        if (manager) manager->pushState(std::make_unique<OptionsState>());
    });

    auto btnMainMenu = std::make_unique<view::ui::UIButton>(
        font, "MAIN MENU", view::ui::layout::ButtonFontSize, sf::Vector2f(0.f, 0.f), sf::Vector2f(view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight));
    btnMainMenu->setOnClick([this]() {
        if (manager) {
            manager->pushState(std::make_unique<WarningPopupState>(
                "Save game and return to Main Menu?",
                WarningPopupState::Type::YesNo,
                [this]() {
                    if (onSaveCallback) {
                        onSaveCallback();
                    }
                    if (manager) {
                        manager->clear();
                        manager->pushState(std::make_unique<MainMenuState>());
                    }
                },
                [this]() {
                    if (manager) {
                        manager->clear();
                        manager->pushState(std::make_unique<MainMenuState>());
                    }
                },
                "YES",
                "NO"
            ));
        }
    });

    menuContainer.add(std::move(btnResume));
    menuContainer.add(std::move(btnRestart));
    menuContainer.add(std::move(btnOptions));
    menuContainer.add(std::move(btnMainMenu));
}

void PauseState::onDisplayModeChanged() {
    float screenW = static_cast<float>(AppEngine::screenWidth());
    float screenH = static_cast<float>(AppEngine::ScreenHeight);
    
    overlay.setSize({screenW, screenH});
    titleLabel.setPosition((screenW - 96.f) / 2.f, 35.f);
    menuContainer.setPosition((screenW - view::ui::layout::MenuButtonWidth) / 2.f, 80.f);
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
    // If user presses the Pause or Back/Escape key again, resume the game
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        const auto& settings = model::SettingsManager::instance().get();
        if (static_cast<int>(key->code) == settings.keyPause ||
            static_cast<int>(key->code) == settings.keyBack ||
            key->code == sf::Keyboard::Key::Escape) {
            model::LogManager::instance().info("Resume");
            if (manager) manager->popState();
            return;
        }
    }
    menuContainer.handleEvent(event);
}

} // namespace controller
