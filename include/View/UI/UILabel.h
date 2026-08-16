#ifndef VIEW_UI_UILABEL_H
#define VIEW_UI_UILABEL_H

#include "View/UI/UIElement.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>

#include <string>

namespace view {
namespace ui {

// A non-interactive text label.
//
// SFML 3 removed sf::Text's default constructor (a font is mandatory). To keep
// UILabel default-constructible (required when stored as a plain member in a
// State class), we store the font pointer + metadata separately and build the
// sf::Text object inside render() each frame. The overhead is negligible for UI
// elements that are drawn at 60 Hz.
class UILabel : public UIElement {
public:
    // Default-constructible — shows nothing until setFont()+setText() are called.
    UILabel() = default;

    UILabel(const sf::Font& font, const std::string& text,
            unsigned int charSize, sf::Color color = sf::Color::White);

    void setFont(const sf::Font& font);
    void setText(const std::string& text);
    void setCharacterSize(unsigned int size);
    void setColor(sf::Color color);

    // If true the string is horizontally centred inside getSize().x.
    void setCentered(bool centred) { centered = centred; }

    void render(sf::RenderTarget& target) override;

private:
    const sf::Font* fontPtr  = nullptr;
    std::string     str;
    unsigned int    charSize = 12u;
    sf::Color       color    = sf::Color::White;
    bool            centered = true;
};

}  // namespace ui
}  // namespace view

#endif
