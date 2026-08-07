#include "View/HudRenderer.h"

#include "View/AssetManager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

#include <string>

namespace view {

HudRenderer::HudRenderer()
    : font(view::AssetManager::instance().getUiFont()),
      fontLoaded(view::AssetManager::instance().isFontLoaded()) {
}

void HudRenderer::render(sf::RenderTarget& window, const HudData& data) const {
    if (!fontLoaded) {
        return;
    }

    // Four equal columns across the logical resolution (640 wide), labels over values.
    constexpr float LabelSize = 16.0f;
    constexpr float ValueSize = 20.0f;
    constexpr float Columns[4] = {24.0f, 216.0f, 400.0f, 528.0f};
    constexpr float LabelY = 10.0f;
    constexpr float ValueY = 36.0f;
    const char* const Labels[4] = {"MARIO", "COINS", "WORLD", "TIME"};

    for (int i = 0; i < 4; ++i) {
        sf::Text label(font, Labels[i], LabelSize);
        label.setFillColor(sf::Color::White);
        label.setOutlineColor(sf::Color::Black);
        label.setOutlineThickness(2.f);
        label.setPosition({Columns[i], LabelY});
        window.draw(label);
    }

    const std::string values[4] = {
        std::to_string(data.score),
        std::to_string(data.coins),
        data.levelName.empty() ? std::string("1-1") : data.levelName,
        std::to_string(data.time),
    };

    for (int i = 0; i < 4; ++i) {
        sf::Text value(font, values[i], ValueSize);
        value.setFillColor(sf::Color::White);
        value.setOutlineColor(sf::Color::Black);
        value.setOutlineThickness(2.f);
        value.setPosition({Columns[i], ValueY});
        window.draw(value);
    }

    // Bottom control hint, below the play area.
    const float hintY = window.getView().getSize().y - 26.0f;
    sf::Text hint(font, "WASD/ARROWS: MOVE  SPACE: JUMP  ESC: MENU", 12);
    hint.setFillColor(sf::Color(220, 220, 220));
    hint.setOutlineColor(sf::Color::Black);
    hint.setOutlineThickness(2.f);
    hint.setPosition({12.0f, hintY});
    window.draw(hint);
}

}
