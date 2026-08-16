#include "View/UI/UIScrollView.h"
#include "View/UI/UITheme.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>
#include <algorithm>

namespace view {
namespace ui {

void UIScrollView::setBounds(const sf::FloatRect& boundsRect) {
    bounds = boundsRect;
    updateScrollbarVisuals();
}

void UIScrollView::setContentHeight(float height) {
    contentHeight = height;
    updateScrollbarVisuals();
}

void UIScrollView::applyScroll(float deltaY) {
    float oldScroll = scrollY;
    scrollY -= deltaY;
    
    float maxScroll = std::max(0.f, contentHeight - bounds.size.y);
    scrollY = std::clamp(scrollY, 0.f, maxScroll);
    
    float actualDelta = oldScroll - scrollY;
    
    // Shift all children
    for (auto& child : children) {
        float cx = child->getPosition().x;
        float cy = child->getPosition().y;
        child->setPosition(cx, cy + actualDelta);
    }
    
    updateScrollbarVisuals();
}

void UIScrollView::updateScrollbarVisuals() {
    if (contentHeight <= bounds.size.y || bounds.size.y == 0.f) {
        scrollbarThumb.setSize({0.f, 0.f}); // Hide scrollbar if no scrolling needed
        return;
    }
    
    float viewRatio = bounds.size.y / contentHeight;
    float thumbHeight = std::max(10.f, bounds.size.y * viewRatio);
    
    float maxScroll = contentHeight - bounds.size.y;
    float scrollRatio = scrollY / maxScroll;
    
    float maxThumbY = bounds.size.y - thumbHeight;
    float thumbY = bounds.position.y + (scrollRatio * maxThumbY);
    
    scrollbarThumb.setSize({4.f, thumbHeight});
    scrollbarThumb.setPosition({bounds.position.x + bounds.size.x - 4.f, thumbY});
    
    if (isMouseInside) {
        scrollbarThumb.setFillColor(view::ui::theme::ScrollbarHovered);
    } else {
        scrollbarThumb.setFillColor(view::ui::theme::ScrollbarNormal);
    }
}

bool UIScrollView::handleEvent(const sf::Event& event) {
    if (!visible) return false;

    if (const auto* scrolled = event.getIf<sf::Event::MouseWheelScrolled>()) {
        sf::Vector2f lp = transformCoordinate(scrolled->position);
        if (bounds.contains(lp)) {
            constexpr float ScrollSpeed = 25.f;
            applyScroll(scrolled->delta * ScrollSpeed);
            return true; // Consume event
        }
    }
    
    // Logical clipping: block events outside bounds and trigger onMouseLeave
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f lp = transformCoordinate(moved->position);
        if (!bounds.contains(lp)) {
            if (isMouseInside) {
                this->onMouseLeave();
                isMouseInside = false;
                updateScrollbarVisuals();
            }
            return false;
        } else {
            if (!isMouseInside) {
                isMouseInside = true;
                updateScrollbarVisuals();
            }
        }
    }
    else if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f lp = transformCoordinate(pressed->position);
        if (!bounds.contains(lp)) return false;
    }
    else if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
        sf::Vector2f lp = transformCoordinate(released->position);
        if (!bounds.contains(lp)) return false;
    }

    return UIContainer::handleEvent(event);
}

void UIScrollView::render(sf::RenderTarget& target) {
    if (!visible) return;

    sf::View oldView = target.getView();
    
    // Create clipped view based on target size
    sf::Vector2f targetSize(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y));
    
    sf::View scrollView = oldView;
    scrollView.setSize(bounds.size);
    scrollView.setCenter({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
    
    sf::FloatRect viewport(
        {bounds.position.x / targetSize.x, bounds.position.y / targetSize.y},
        {bounds.size.x / targetSize.x, bounds.size.y / targetSize.y}
    );
    scrollView.setViewport(viewport);
    
    target.setView(scrollView);
    
    // Render children
    for (auto& child : children) {
        child->render(target);
    }
    
    // Restore original view
    target.setView(oldView);
    
    // Render scrollbar on top of the clipped view, using the original target view
    if (scrollbarThumb.getSize().y > 0.f) {
        target.draw(scrollbarThumb);
    }
}

} // namespace ui
} // namespace view
