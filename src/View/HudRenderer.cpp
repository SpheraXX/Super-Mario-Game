#include "View/HudRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

namespace view {

HudRenderer::HudRenderer() : fontLoaded(font.openFromFile("assets/fonts/Tuffy.ttf")) {
}

void HudRenderer::render(sf::RenderTarget& window) const {
    if (!fontLoaded) return;

    sf::Text levelLabel(font, "DEBUG LEVEL", 22);
    levelLabel.setFillColor(sf::Color::White);
    levelLabel.setOutlineColor(sf::Color::Black);
    levelLabel.setOutlineThickness(2.f);
    levelLabel.setPosition({10.f, 8.f});
    window.draw(levelLabel);

    // The HUD is drawn on the fixed view, whose size is the logical resolution.
    const float hintY = window.getView().getSize().y - 26.0f;
    sf::Text hint(font, "WASD/Arrows: Move  |  Space/W/Up: Jump  |  ESC: Menu", 16);
    hint.setFillColor(sf::Color::White);
    hint.setOutlineColor(sf::Color::Black);
    hint.setOutlineThickness(2.f);
    hint.setPosition({10.f, hintY});
    window.draw(hint);
}

}
