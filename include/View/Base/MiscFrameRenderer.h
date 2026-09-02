#ifndef VIEW_MISCFRAMERENDERER_H
#define VIEW_MISCFRAMERENDERER_H

#include "View/Base/MiscAtlas.h"
#include "View/Base/SpriteEntityRenderer.h"

namespace view {

// AtlasFrameRenderer's twin for assets/misc.png: draws an entity as one fixed frame off
// that sheet, keyed on black. Separate from AtlasFrameRenderer rather than a parameter on
// it because the sheet and its colour key travel together — a renderer that took both as
// arguments would let a caller pair misc.png with the enemy sheet's key and silently get
// an unmasked backdrop.
template <typename T>
class MiscFrameRenderer : public SpriteEntityRenderer<T> {
public:
    explicit MiscFrameRenderer(sf::IntRect frame)
        : SpriteEntityRenderer<T>(atlas::MiscSheet, atlas::MiscColorKey), frame(frame) {}

protected:
    void renderTyped(sf::RenderTarget& window, const T& entity,
                     const RenderContext& /* ctx */) const override {
        this->drawCharacterFrame(window, entity, frame);
    }

    bool flipWhenDying() const override { return true; }

private:
    sf::IntRect frame;
};

}

#endif
