#include "View/HudRenderer.h"

#include "View/AssetManager.h"
#include "View/TextUtils.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

#include <string>

namespace view {

namespace {
// One size for everything on the bar: equal glyphs + monospaced advance = equidistant
// columns. Values sit under their label at the same anchor, so the grid never wanders.
constexpr float FontSize = 16.0f;
constexpr float Outline = 1.0f;
constexpr float Columns[4] = {24.0f, 216.0f, 400.0f, 528.0f};
constexpr float LabelY = 8.0f;
constexpr float ValueY = 32.0f;
const char* const Labels[4] = {"MARIO", "COINS", "WORLD", "TIME"};
const char* const HintString = "WASD/ARROWS: MOVE   SPACE: JUMP   ESC: MENU";
}

HudRenderer::HudRenderer()
    : font(view::AssetManager::instance().getUiFont()),
      fontLoaded(view::AssetManager::instance().isFontLoaded()) {
    hintSize = text::fitCharacterSize(font, HintString, 616.0f, 12);
}

void HudRenderer::render(sf::RenderTarget& window, const HudData& data) const {
    if (!fontLoaded) {
        return;
    }

    for (int i = 0; i < 4; ++i) {
        sf::Text label(font, Labels[i], FontSize);
        label.setFillColor(sf::Color::White);
        label.setOutlineColor(sf::Color::Black);
        label.setOutlineThickness(Outline);
        label.setPosition(text::snap({Columns[i], LabelY}));
        window.draw(label);
    }

    const std::string values[4] = {
        std::to_string(data.score),
        std::to_string(data.coins),
        data.levelName.empty() ? std::string("1-1") : data.levelName,
        std::to_string(data.time),
    };

    for (int i = 0; i < 4; ++i) {
        sf::Text value(font, values[i], FontSize);
        value.setFillColor(sf::Color::White);
        value.setOutlineColor(sf::Color::Black);
        value.setOutlineThickness(Outline);
        value.setPosition(text::snap({Columns[i], ValueY}));
        window.draw(value);
    }

    // Bottom control hint, below the play area, fitted so it never runs off the screen.
    const float hintY = window.getView().getSize().y - 24.0f;
    sf::Text hint(font, HintString, hintSize);
    hint.setFillColor(sf::Color(220, 220, 220));
    hint.setOutlineColor(sf::Color::Black);
    hint.setOutlineThickness(Outline);
    hint.setPosition(text::snap({12.0f, hintY}));
    window.draw(hint);
}

}
