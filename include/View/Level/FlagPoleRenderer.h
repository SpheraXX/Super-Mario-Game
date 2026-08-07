#ifndef VIEW_LEVEL_FLAGPOLERENDERER_H
#define VIEW_LEVEL_FLAGPOLERENDERER_H

#include "View/Base/EntityRenderer.h"

#include <SFML/Graphics/Texture.hpp>

namespace model {
class FlagPole;
}

namespace view {

// Temporary placeholder art for the flagpole, cropped from the existing blocks.png
// atlas (all rectangles are multiples of 16): a teal tile stretched into the pole, a
// gold tile for the ball on top and the pennant.
class FlagPoleRenderer : public TypedEntityRenderer<model::FlagPole> {
public:
    FlagPoleRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::FlagPole& pole) const override;

private:
    sf::Texture texture;
    bool textureLoaded;
};

}

#endif
