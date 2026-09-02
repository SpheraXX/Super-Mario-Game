#ifndef VIEW_UI_SOLIDBUTTONSKIN_H
#define VIEW_UI_SOLIDBUTTONSKIN_H

#include "View/UI/IButtonSkin.h"
#include <SFML/Graphics/RectangleShape.hpp>

namespace view {
namespace ui {

class SolidButtonSkin : public IButtonSkin {
public:
    SolidButtonSkin();
    
    void setPosition(float x, float y) override;
    void setSize(float w, float h) override;
    
    void updateState(bool hovered, bool enabled) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void setColors(sf::Color normal, sf::Color hovered) override;
    void setKeepTransparentWhenDisabled(bool keep) override;

private:
    sf::RectangleShape background;
    sf::Color colorNormal;
    sf::Color colorHovered;
    bool isHovered = false;
    bool isEnabled = true;
    bool keepTransparentWhenDisabled = false;
};

} // namespace ui
} // namespace view

#endif
