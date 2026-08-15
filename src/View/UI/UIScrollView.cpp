#include "View/UI/UIScrollView.h"
#include "Controller/AppEngine.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>
#include <algorithm>

namespace view {
namespace ui {

void UIScrollView::setBounds(const sf::FloatRect& boundsRect) {
    bounds = boundsRect;
}

void UIScrollView::setContentHeight(float height) {
    contentHeight = height;
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
}

bool UIScrollView::handleEvent(const sf::Event& event) {
    if (!visible) return false;

    if (const auto* scrolled = event.getIf<sf::Event::MouseWheelScrolled>()) {
        sf::Vector2f lp = controller::AppEngine::windowToLogical(scrolled->position);
        if (bounds.contains(lp)) {
            constexpr float ScrollSpeed = 25.f;
            applyScroll(scrolled->delta * ScrollSpeed);
            return true; // Consume event
        }
    }
    
    // Logical clipping: block events outside bounds and trigger onMouseLeave
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f lp = controller::AppEngine::windowToLogical(moved->position);
        if (!bounds.contains(lp)) {
            if (isMouseInside) {
                this->onMouseLeave();
                isMouseInside = false;
            }
            return false;
        } else {
            isMouseInside = true;
        }
    }
    else if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f lp = controller::AppEngine::windowToLogical(pressed->position);
        if (!bounds.contains(lp)) return false;
    }
    else if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
        sf::Vector2f lp = controller::AppEngine::windowToLogical(released->position);
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
}

} // namespace ui
} // namespace view
