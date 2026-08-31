#ifndef VIEW_UI_UIKEYICON_H
#define VIEW_UI_UIKEYICON_H

#include "View/UI/UIElement.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <string>

namespace view {
namespace ui {

// Displays a pixel-art key icon for any SFML keyboard key.
//
// NOTE: UIKeyIcon is intentionally NOT IClickable.
// Click interaction is handled by an overlapping transparent UIButton
// (mask overlay pattern). UIKeyIcon only manages visual appearance.
class UIKeyIcon : public UIElement {
public:
    UIKeyIcon();
    explicit UIKeyIcon(sf::Keyboard::Key key);

    void setKey(sf::Keyboard::Key key);
    sf::Keyboard::Key getKey() const;

    // Static helper to map SFML key to file path
    static std::string keyToPath(sf::Keyboard::Key key);

    // Override IDrawable
    void render(sf::RenderTarget& target) override;

    // Override UIElement
    void setPosition(float x, float y) override;
    
    void setScale(float scaleX, float scaleY);
    void setColor(sf::Color color);

private:
    void updateTexture();

    sf::Keyboard::Key currentKey;
    sf::Sprite sprite;
};

}  // namespace ui
}  // namespace view

#endif
