#ifndef VIEW_UI_UICONTAINER_H
#define VIEW_UI_UICONTAINER_H

#include "View/UI/UIElement.h"

#include <memory>
#include <vector>

namespace view {
namespace ui {

// A layout container that owns and dispatches to a list of child UIElements.
//
// This is the Composite Pattern node. UIContainer IS a UIElement (it can be
// nested inside another UIContainer) and it HAS UIElements (its children).
//
// Layout modes:
//   NONE      — children placed at their own absolute positions (default).
//   VERTICAL  — children stacked top-to-bottom with a configurable gap.
//
// LSP: UIContainer replaces UIElement anywhere in a tree. render/update/
// handleEvent are dispatched to every child that is visible/active.
class UIContainer : public UIElement {
public:
    enum class Layout { None, Vertical };

    explicit UIContainer(Layout layout = Layout::None, float gap = 0.f);

    // Takes ownership of the child element. Returns a raw pointer for immediate
    // post-add setup (e.g. setting colors after emplacing a button).
    // Note: The child's initial position is treated as relative to this container's
    // current position.
    template<typename T>
    T* add(std::unique_ptr<T> child) {
        T* raw = child.get();
        
        // Shift the child by the container's absolute position so that the initial
        // coordinates provided by the user act as local/relative coordinates.
        raw->setPosition(raw->getPosition().x + pos.x, raw->getPosition().y + pos.y);
        
        children.push_back(std::move(child));
        if (currentLayout != Layout::None) relayout();
        return raw;
    }

    virtual void clear();
    bool empty() const { return children.empty(); }

    // ── UIElement overrides ──────────────────────────────────────────────────
    void setPosition(float x, float y) override;
    void render(sf::RenderTarget& target) override;
    void update(float deltaTime) override;
    bool handleEvent(const sf::Event& event) override;
    void onMouseLeave() override;

    // Re-runs the layout algorithm. Call after adding all children or changing
    // size, if you use an automatic layout mode.
    void relayout();

protected:
    std::vector<std::unique_ptr<UIElement>> children;

private:
    Layout  currentLayout;
    float   gap;           // pixels between items (Vertical mode)
};

}  // namespace ui
}  // namespace view

#endif
