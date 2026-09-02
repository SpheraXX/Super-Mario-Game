#include "View/UI/UILabel.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <cmath>

namespace view {
namespace ui {

UILabel::UILabel(const sf::Font& font, const std::string& text,
                 unsigned int cs, sf::Color col)
    : fontPtr(&font), str(text), charSize(cs), color(col) {
}

void UILabel::setFont(const sf::Font& font)     { fontPtr  = &font; }
void UILabel::setText(const std::string& text)  { str      = text;  }
void UILabel::setCharacterSize(unsigned int cs) { charSize = cs;    }
void UILabel::setColor(sf::Color col)           { color    = col;   }

void UILabel::render(sf::RenderTarget& target) {
    if (!visible || !fontPtr || str.empty()) return;

    sf::Text sfText(*fontPtr, str, charSize);
    sfText.setFillColor(color);
    sfText.setLineSpacing(lineSpacing);

    const sf::FloatRect bounds = sfText.getLocalBounds();
    float x = pos.x;
    if (centered && size.x > 0.f) {
        x = pos.x + (size.x - bounds.size.x) / 2.f - bounds.position.x;
    }
    // Snap to whole pixel so pixel font glyphs stay aligned on the offscreen buffer.
    sfText.setPosition({std::floor(x), std::floor(pos.y)});
    target.draw(sfText);
}

}  // namespace ui
}  // namespace view
