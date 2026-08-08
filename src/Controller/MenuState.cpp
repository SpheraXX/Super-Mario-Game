#include "Controller/MenuState.h"

#include "Controller/AppEngine.h"
#include "Controller/PlayState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "View/AssetManager.h"
#include "View/TextUtils.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>

namespace controller {

void MenuState::onEnter() {
    // Entering the menu means a fresh run: clear score/lives/level.
    model::GameManager::instance().reset();
    font = view::AssetManager::instance().getUiFont();
    fontLoaded = view::AssetManager::instance().isFontLoaded();
    if (fontLoaded) {
        titleSize = view::text::fitCharacterSize(font, "SUPER MARIO", 600.0f, 56);
        startHintSize = view::text::fitCharacterSize(font, "Press ENTER or SPACE to Start", 600.0f, 24);
        quitHintSize = view::text::fitCharacterSize(font, "Press ESC to Quit", 600.0f, 20);
    }
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
    window.clear(sf::Color(20, 20, 60));

    if (!fontLoaded) {
        return;
    }

    const float centerX = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;
    const float centerY = static_cast<float>(AppEngine::ScreenHeight);

    sf::Text title(font, "SUPER MARIO", titleSize);
    title.setFillColor(sf::Color(230, 90, 30));
    view::text::drawCentered(window, title, centerX, centerY * 0.35f);

    sf::Text startHint(font, "Press ENTER or SPACE to Start", startHintSize);
    startHint.setFillColor(sf::Color::White);
    view::text::drawCentered(window, startHint, centerX, centerY * 0.55f);

    sf::Text quitHint(font, "Press ESC to Quit", quitHintSize);
    quitHint.setFillColor(sf::Color(200, 200, 200));
    view::text::drawCentered(window, quitHint, centerX, centerY * 0.65f);
}

}
