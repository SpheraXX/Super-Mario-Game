#include "Controller/MenuState.h"

#include "Controller/AppEngine.h"
#include "Controller/PlayState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "View/AssetManager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>
#include <iostream>

namespace controller {

namespace {
void centerOrigin(sf::Text& text) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
}
}

void MenuState::onEnter() {
    // Entering the menu means a fresh run: clear score/lives/level.
    model::GameManager::instance().reset();
    font = view::AssetManager::instance().getUiFont();
    fontLoaded = view::AssetManager::instance().isFontLoaded();
}

void MenuState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        switch (key->code) {
            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space:
                manager->replaceState(std::make_unique<PlayState>());
                break;
            case sf::Keyboard::Key::Escape:
                manager->clear(); // empties the stack -> AppEngine loop exits
                break;
            default:
                break;
        }
    }
}

void MenuState::update(float deltaTime) {
    (void)deltaTime;
}

void MenuState::render(sf::RenderTarget& window) {
    std :: cerr << "Rendering MenuState" << std :: endl;
    window.clear(sf::Color(20, 20, 60));

    if (!fontLoaded) {
        return;
    }

    const float centerX = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;
    const float centerY = static_cast<float>(AppEngine::ScreenHeight);

    sf::Text title(font, "SUPER MARIO", 56);
    title.setFillColor(sf::Color(230, 90, 30));
    centerOrigin(title);
    title.setPosition({centerX, centerY * 0.35f});
    window.draw(title);

    sf::Text startHint(font, "Press ENTER or SPACE to Start", 24);
    startHint.setFillColor(sf::Color::White);
    centerOrigin(startHint);
    startHint.setPosition({centerX, centerY * 0.55f});
    window.draw(startHint);

    sf::Text quitHint(font, "Press ESC to Quit", 20);
    quitHint.setFillColor(sf::Color(200, 200, 200));
    centerOrigin(quitHint);
    quitHint.setPosition({centerX, centerY * 0.65f});
    window.draw(quitHint);
}

}
