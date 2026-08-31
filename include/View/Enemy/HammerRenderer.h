#ifndef VIEW_ENEMY_HAMMERRENDERER_H
#define VIEW_ENEMY_HAMMERRENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

namespace model {
class Hammer;
}

namespace view {

// Draws a thrown hammer as a four-frame spin off misc.png.
//
// The four poses are not a common size — the two horizontal ones are 14x8 and the two
// vertical ones 8x16 — so each is drawn into a box of its own shape, centred on the
// hammer's 16x16 collision box, instead of being stretched onto it. Stretching would make
// the head swell and shrink through the turn, and the horizontal poses would come out
// twice their proper height.
class HammerRenderer : public SpriteEntityRenderer<model::Hammer> {
public:
    HammerRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::Hammer& hammer,
                     const RenderContext& ctx) const override;

private:
    // A full turn takes this long. Fast enough to read as a tumble at the speed a hammer
    // crosses the screen, slow enough that the four poses stay distinguishable.
    static constexpr float SpinSeconds = 0.32f;
};

}

#endif
