#include "View/UI/UIKeyIcon.h"
#include "View/AssetManager.h"
#include <cctype>

namespace view {
namespace ui {

UIKeyIcon::UIKeyIcon() 
    : currentKey(sf::Keyboard::Key::Unknown)
    , sprite(AssetManager::instance().getTexture(keyToPath(sf::Keyboard::Key::Unknown))) {
    updateTexture();
}

UIKeyIcon::UIKeyIcon(sf::Keyboard::Key key) 
    : currentKey(key)
    , sprite(AssetManager::instance().getTexture(keyToPath(key))) {
    updateTexture();
}

void UIKeyIcon::setKey(sf::Keyboard::Key key) {
    currentKey = key;
    updateTexture();
}

sf::Keyboard::Key UIKeyIcon::getKey() const {
    return currentKey;
}

void UIKeyIcon::setPosition(float x, float y) {
    UIElement::setPosition(x, y);
    sprite.setPosition(pos);
}

void UIKeyIcon::setScale(float scaleX, float scaleY) {
    sprite.setScale({scaleX, scaleY});
    // Update the logical size based on the scaled texture bounds
    const sf::FloatRect bounds = sprite.getGlobalBounds();
    size = {bounds.size.x, bounds.size.y};
}

void UIKeyIcon::setColor(sf::Color color) {
    sprite.setColor(color);
}

void UIKeyIcon::updateTexture() {
    std::string path = keyToPath(currentKey);
    const sf::Texture& tex = AssetManager::instance().getTexture(path);
    sprite.setTexture(tex, true);
    
    // Automatically update the UIElement size based on the texture size
    const sf::FloatRect bounds = sprite.getGlobalBounds();
    size = {bounds.size.x, bounds.size.y};
    sprite.setPosition(pos);
}

void UIKeyIcon::render(sf::RenderTarget& target) {
    if (!visible) return;
    target.draw(sprite);
}

std::string UIKeyIcon::keyToPath(sf::Keyboard::Key key) {
    // Base directory for white pixel key assets
    static constexpr const char* kKeyIconBase =
        "resources/Pixel Keys x16/Tiles White/pxkw_";
    const std::string base = kKeyIconBase;
    
    if (key >= sf::Keyboard::Key::A && key <= sf::Keyboard::Key::Z) {
        char c = 'a' + (static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::A));
        return base + c + ".png";
    }
    if (key >= sf::Keyboard::Key::Num0 && key <= sf::Keyboard::Key::Num9) {
        char c = '0' + (static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::Num0));
        return base + c + ".png";
    }
    
    switch (key) {
        case sf::Keyboard::Key::Escape:    return base + "escape.png";
        case sf::Keyboard::Key::LControl:  return base + "control.png";
        case sf::Keyboard::Key::LShift:    return base + "shift.png";
        case sf::Keyboard::Key::LAlt:      return base + "alt.png";
        case sf::Keyboard::Key::LSystem:   return base + "windows.png";
        case sf::Keyboard::Key::RControl:  return base + "control.png";
        case sf::Keyboard::Key::RShift:    return base + "shift.png";
        case sf::Keyboard::Key::RAlt:      return base + "alt.png";
        case sf::Keyboard::Key::RSystem:   return base + "windows.png";
        case sf::Keyboard::Key::Menu:      return base + "menu.png";
        case sf::Keyboard::Key::LBracket:  return base + "bracket_left.png";
        case sf::Keyboard::Key::RBracket:  return base + "bracket_right.png";
        case sf::Keyboard::Key::Semicolon: return base + "semicolon.png";
        case sf::Keyboard::Key::Comma:     return base + "comma.png";
        case sf::Keyboard::Key::Period:    return base + "dot.png";
        case sf::Keyboard::Key::Apostrophe:return base + "quote.png";
        case sf::Keyboard::Key::Slash:     return base + "forwardslash.png";
        case sf::Keyboard::Key::Backslash: return base + "backslash.png";
        case sf::Keyboard::Key::Grave:     return base + "tilde.png";
        case sf::Keyboard::Key::Equal:     return base + "equal.png";
        case sf::Keyboard::Key::Hyphen:    return base + "dash.png";
        case sf::Keyboard::Key::Space:     return base + "space_2.png";
        case sf::Keyboard::Key::Enter:     return base + "enter.png";
        case sf::Keyboard::Key::Backspace: return base + "backspace.png";
        case sf::Keyboard::Key::Tab:       return base + "tab.png";
        case sf::Keyboard::Key::PageUp:    return base + "page_up.png";
        case sf::Keyboard::Key::PageDown:  return base + "page_down.png";
        case sf::Keyboard::Key::End:       return base + "end.png";
        case sf::Keyboard::Key::Home:      return base + "home.png";
        case sf::Keyboard::Key::Insert:    return base + "insert.png";
        case sf::Keyboard::Key::Delete:    return base + "delete.png";
        case sf::Keyboard::Key::Add:       return base + "plus.png";
        case sf::Keyboard::Key::Subtract:  return base + "dash.png";
        case sf::Keyboard::Key::Multiply:  return base + "asterisk.png";
        case sf::Keyboard::Key::Divide:    return base + "forwardslash.png";
        case sf::Keyboard::Key::Left:      return base + "arrow_left.png";
        case sf::Keyboard::Key::Right:     return base + "arrow_right.png";
        case sf::Keyboard::Key::Up:        return base + "arrow_up.png";
        case sf::Keyboard::Key::Down:      return base + "arrow_down.png";
        default: break;
    }
    
    // For function keys F1-F12
    if (key >= sf::Keyboard::Key::F1 && key <= sf::Keyboard::Key::F12) {
        int fNum = 1 + (static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::F1));
        return base + "f" + std::to_string(fNum) + ".png";
    }
    
    // Fallback: If unknown key, return empty
    return base + "empty.png";
}

}  // namespace ui
}  // namespace view
