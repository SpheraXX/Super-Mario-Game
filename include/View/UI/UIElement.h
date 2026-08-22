#ifndef VIEW_UI_UIELEMENT_H
#define VIEW_UI_UIELEMENT_H

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Vector2.hpp>

#include <functional>

namespace view {
namespace ui {

// ─── IDrawable ────────────────────────────────────────────────────────────────
// ISP: every UIElement can be rendered, but only interactive ones implement
// IClickable. Separating the interfaces prevents non-interactive Labels from
// being forced to stub out onClick/onHover.
struct IDrawable {
    virtual void render(sf::RenderTarget& target) = 0;
    virtual ~IDrawable() = default;
};

// ─── IClickable ───────────────────────────────────────────────────────────────
// ISP: only interactive widgets (UIButton, UISlider) implement this.
struct IClickable {
    virtual void onHover(bool hovered) = 0;
    virtual void onClick()             = 0;
    virtual ~IClickable()              = default;
};

// ─── UIElement (base) ─────────────────────────────────────────────────────────
// Root of the Composite tree. Every on-screen object inherits from here.
//
// Design notes (SOLID):
//   SRP  — UIElement is only responsible for position/size and the render/update
//           lifecycle. It does NOT load assets, does NOT run game logic.
//   OCP  — New widget types (UISlider, UICheckbox, UIVideoPlayer) are created by
//           extending this class, never by modifying UIManager or UIContainer.
//   LSP  — UIContainer holds vector<UIElement*>; any derived type plugs in and
//           render()/update() are called polymorphically without special-casing.
class UIElement : public IDrawable {
public:
    virtual ~UIElement() = default;

    // ── Coordinate Bridge (DIP) ───────────────────────────────────────────────
    // Injected once by AppEngine at startup. Converts window-pixel coordinates
    // to logical game coordinates. UI widgets call this instead of importing
    // AppEngine directly, keeping the View layer free of Controller dependencies.
    static std::function<sf::Vector2f(const sf::Vector2i&)> transformCoordinate;

    // Called once per frame. Override to animate (e.g. button pulse, scroll).
    virtual void update(float deltaTime) { (void)deltaTime; }

    // Forward the raw SFML event into the widget tree. Return true to signal
    // the event was consumed and should not propagate further.
    virtual bool handleEvent(const sf::Event& event) {
        (void)event;
        return false;
    }

    // Called when the mouse leaves the bounds of a parent clipping container
    virtual void onMouseLeave() {}

    // ── Geometry ──────────────────────────────────────────────────────────────
    virtual void setPosition(float x, float y) { pos = {x, y}; }
    virtual void setSize(float w, float h)     { size = {w, h}; }
    sf::Vector2f getPosition() const          { return pos; }
    sf::Vector2f getSize()     const          { return size; }

    bool isVisible() const           { return visible; }
    void setVisible(bool v)          { visible = v; }

    bool isEnabled() const           { return enabled; }
    void setEnabled(bool e)          { enabled = e; }

protected:
    // Check if a window-space point is inside this element's bounding box.
    bool contains(float x, float y) const {
        return x >= pos.x && x <= pos.x + size.x &&
               y >= pos.y && y <= pos.y + size.y;
    }

    sf::Vector2f pos  = {0.f, 0.f};
    sf::Vector2f size = {0.f, 0.f};
    bool visible      = true;
    bool enabled      = true;
};

}  // namespace ui
}  // namespace view

#endif
