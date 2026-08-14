#include "View/UI/UIContainer.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace view {
namespace ui {

UIContainer::UIContainer(Layout layout, float gap)
    : currentLayout(layout), gap(gap) {
}

void UIContainer::clear() {
    children.clear();
}

void UIContainer::relayout() {
    if (currentLayout == Layout::None) return;

    // Vertical stacking: lay children top-to-bottom within [pos.x, pos.x+size.x],
    // starting at pos.y.
    float cursor = pos.y;
    for (auto& child : children) {
        child->setPosition(pos.x, cursor);
        cursor += child->getSize().y + gap;
    }
}

void UIContainer::render(sf::RenderTarget& target) {
    if (!visible) return;
    for (auto& child : children) {
        child->render(target);
    }
}

void UIContainer::update(float deltaTime) {
    if (!visible) return;
    for (auto& child : children) {
        child->update(deltaTime);
    }
}

bool UIContainer::handleEvent(const sf::Event& event) {
    if (!visible) return false;
    // Dispatch to children in reverse order (topmost widget gets priority).
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->handleEvent(event)) {
            return true;  // event consumed — stop propagation
        }
    }
    return false;
}

}  // namespace ui
}  // namespace view
