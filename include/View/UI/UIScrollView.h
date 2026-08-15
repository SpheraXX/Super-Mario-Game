#ifndef VIEW_UI_UISCROLLVIEW_H
#define VIEW_UI_UISCROLLVIEW_H

#include "View/UI/UIContainer.h"
#include <SFML/Graphics/Rect.hpp>

namespace view {
namespace ui {

class UIScrollView : public UIContainer {
public:
    UIScrollView() = default;

    void setBounds(const sf::FloatRect& boundsRect);
    void setContentHeight(float height);

    void render(sf::RenderTarget& target) override;
    bool handleEvent(const sf::Event& event) override;

private:
    sf::FloatRect bounds;
    float contentHeight = 0.f;
    float scrollY = 0.f;
    bool isMouseInside = false;

    void applyScroll(float deltaY);
};

} // namespace ui
} // namespace view

#endif
