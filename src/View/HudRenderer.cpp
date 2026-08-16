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
constexpr float FontSize = 8.0f;
constexpr float Outline = 1.0f;
// Columns as FRACTIONS of the logical width rather than absolute pixels. The logical frame
// is only as wide as the window's aspect ratio allows (see AppEngine::screenWidth), so a
// fixed 528px column that fitted the old 640-wide frame would sit off the right edge of a
// 384-wide one. Fractions keep the same spacing at every width.
constexpr float ColumnFractions[4] = {0.0375f, 0.3375f, 0.625f, 0.825f};
constexpr float LabelY = 4.0f;
constexpr float ValueY = 16.0f;
const char* const Labels[4] = {"MARIO", "COINS", "WORLD", "TIME"};
const char* const HintString = "WASD/ARROWS: MOVE   SPACE: JUMP   ESC: MENU";
}

HudRenderer::HudRenderer()
    : font(view::AssetManager::instance().getUiFont()),
      fontLoaded(view::AssetManager::instance().isFontLoaded()) {
}

void HudRenderer::render(sf::RenderTarget& window, const HudData& data) const {
    if (!fontLoaded) {
        return;
    }

    // Everything is laid out against the view actually in force, so cycling the window size
    // (which changes how many columns the logical frame holds) re-fits the bar for free.
    const float viewWidth = window.getView().getSize().x;
    // Re-fit the hint only when the width actually changes: fitCharacterSize measures the
    // string once per candidate size, which is not work to repeat every frame.
    if (viewWidth != fittedForWidth) {
        hintSize = text::fitCharacterSize(font, HintString, viewWidth - 12.0f, 6);
        fittedForWidth = viewWidth;
    }

    for (int i = 0; i < 4; ++i) {
        sf::Text label(font, Labels[i], FontSize);
        label.setFillColor(sf::Color::White);
        label.setOutlineColor(sf::Color::Black);
        label.setOutlineThickness(Outline);
        label.setPosition(text::snap({ColumnFractions[i] * viewWidth, LabelY}));
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
        value.setPosition(text::snap({ColumnFractions[i] * viewWidth, ValueY}));
        window.draw(value);
    }

    // Bottom control hint, below the play area, fitted so it never runs off the screen.
    const float hintY = window.getView().getSize().y - 12.0f;
    sf::Text hint(font, HintString, hintSize);
    hint.setFillColor(sf::Color(220, 220, 220));
    hint.setOutlineColor(sf::Color::Black);
    hint.setOutlineThickness(Outline);
    hint.setPosition(text::snap({6.0f, hintY}));
    window.draw(hint);
}

}
