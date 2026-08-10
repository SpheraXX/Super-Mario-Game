#include "View/HudRenderer.h"

#include "Model/Core/GameManager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace view {

namespace {
// Zero-padded so the counter keeps a stable width and does not jitter as it grows.
std::string padded(int value, int width) {
    std::ostringstream out;
    out << std::setfill('0') << std::setw(width) << value;
    return out.str();
}
}

HudRenderer::HudRenderer() : fontLoaded(font.openFromFile("assets/fonts/Tuffy.ttf")) {
}

void HudRenderer::drawText(sf::RenderTarget& window, const std::string& content,
                           float x, float y, unsigned int size, bool rightAligned) const {
    sf::Text text(font, content, size);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(2.f);

    if (rightAligned) {
        // getLocalBounds carries the glyph's own left bearing, so the right edge is
        // position + size, not size alone. Ignoring that leaves the column ragged.
        const sf::FloatRect bounds = text.getLocalBounds();
        x -= bounds.position.x + bounds.size.x;
    }
    text.setPosition({std::round(x), std::round(y)});
    window.draw(text);
}

void HudRenderer::render(sf::RenderTarget& window) const {
    if (!fontLoaded) return;

    const model::GameManager& game = model::GameManager::instance();

    // The HUD is drawn on the fixed view, whose size is the logical resolution.
    const sf::Vector2f viewSize = window.getView().getSize();

    drawText(window, "DEBUG LEVEL", 10.f, 8.f, 22, false);

    // Counters stack down the top-right corner, right-aligned against a shared margin so
    // the numbers line up as they change width.
    const float right = viewSize.x - 10.f;
    float y = 8.f;
    const float lineHeight = 20.f;

    drawText(window, "SCORE " + padded(game.getScore(), 6), right, y, 18, true);
    y += lineHeight;
    drawText(window, "COINS x" + padded(game.getCoins(), 2), right, y, 18, true);
    y += lineHeight;
    drawText(window, "LIVES x" + padded(game.getLives(), 2), right, y, 18, true);
    y += lineHeight;
    drawText(window, "TIME  " + padded(game.getTimeRemaining(), 3), right, y, 18, true);

    const float hintY = viewSize.y - 26.0f;
    drawText(window, "WASD/Arrows: Move  |  C: Switch  |  X: Shoot  |  Space/W/Up: Jump  |  ESC: Menu", 10.f, hintY, 16,
             false);
}

}
