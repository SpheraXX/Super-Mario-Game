#ifndef VIEW_LEVEL_SLIDERRENDERER_H
#define VIEW_LEVEL_SLIDERRENDERER_H

#include "View/Base/EntityRenderer.h"
#include "View/Base/SpritePainter.h"

namespace model {
class Slider;
}

namespace view {

// Draws a Slider from items.png: a fixed 32x8 platform, anchored to the TOP of the
// entity's (slightly taller) collision box so the art sits exactly where a rider's feet
// land. The extra collision depth below the art is a tunnelling margin, not a visible gap
// — see Slider's construction in LevelScene for why the hitbox is deliberately thicker
// than the sprite.
class SliderRenderer : public TypedEntityRenderer<model::Slider> {
public:
    SliderRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::Slider& slider,
                     const RenderContext& ctx) const override;

private:
    SpritePainter painter;
};

}

#endif
