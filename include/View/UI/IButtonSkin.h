#ifndef VIEW_UI_IBUTTONSKIN_H
#define VIEW_UI_IBUTTONSKIN_H

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Color.hpp>

namespace view {
namespace ui {

class IButtonSkin {
public:
    virtual ~IButtonSkin() = default;

    virtual void setPosition(float x, float y) = 0;
    virtual void setSize(float w, float h) = 0;
    
    virtual void updateState(bool hovered, bool enabled) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    virtual void setColors(sf::Color normal, sf::Color hovered) = 0;

    // When true, the skin remains fully transparent even when disabled.
    // Use this for invisible mask buttons overlaid on top of other widgets.
    virtual void setKeepTransparentWhenDisabled(bool keep) = 0;
};

} // namespace ui
} // namespace view

#endif
