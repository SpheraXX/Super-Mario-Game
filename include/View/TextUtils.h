#ifndef VIEW_TEXTUTILS_H
#define VIEW_TEXTUTILS_H

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <cmath>
#include <string>

namespace view {
namespace text {

// Snap a position to whole pixels so the glyph texels stay aligned: a fractional offset
// re-samples the glyph atlas between texels and smears the edges of a pixel font.
inline sf::Vector2f snap(const sf::Vector2f& position) {
    return {std::floor(position.x), std::floor(position.y)};
}

// Draw a text centered on (centerX, centerY) at a whole-pixel position.
inline void drawCentered(sf::RenderTarget& window, const sf::Text& text,
                         float centerX, float centerY) {
    const sf::FloatRect bounds = text.getLocalBounds();
    sf::Text copy = text;
    copy.setPosition(snap({centerX - bounds.size.x / 2.0f, centerY - bounds.size.y / 2.0f}));
    window.draw(copy);
}

// The largest character size (<= preferredSize) whose rendered width still fits within
// maxWidth. Prevents menu/overlay strings from running off the fixed logical screen.
unsigned int fitCharacterSize(const sf::Font& font, const std::string& content,
                              float maxWidth, unsigned int preferredSize);

}
}

#endif
