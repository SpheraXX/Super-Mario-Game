#ifndef VIEW_ITEMFRAMERENDERER_H
#define VIEW_ITEMFRAMERENDERER_H

#include "View/Base/SpriteEntityRenderer.h"
#include "View/Item/ItemAtlas.h"

namespace view {

// Draws an item as a single fixed frame from the item spritesheet. Every item currently
// has exactly one pose, so they all share this template; a type with real state later
// (e.g. an animating Star) can swap to its own renderer without touching the model.
//
// The frame is a constructor argument rather than a template parameter so all rects in
// use stay visible together at registration time, next to the atlas that defines them.
template <typename T>
class ItemFrameRenderer : public SpriteEntityRenderer<T> {
public:
    explicit ItemFrameRenderer(sf::IntRect frame)
        : SpriteEntityRenderer<T>(atlas::ItemSheet, /*sourceFacesRight=*/true), frame(frame) {}

protected:
    void renderTyped(sf::RenderTarget& window, const T& entity) const override {
        this->drawCharacterFrame(window, entity, frame);
    }

private:
    sf::IntRect frame;
};

}

#endif
