#ifndef VIEW_LEVEL_CASTLERENDERER_H
#define VIEW_LEVEL_CASTLERENDERER_H

#include "View/Base/EntityRenderer.h"

#include <SFML/Graphics/Texture.hpp>

namespace model {
class Castle;
}

namespace view {

// Temporary placeholder art for the goal castle: a grid of brick tiles cropped from
// the existing blocks.png atlas with a darker teal doorway, all in 16px source tiles.
class CastleRenderer : public TypedEntityRenderer<model::Castle> {
public:
    CastleRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::Castle& castle,
                     const RenderContext& ctx) const override;

private:
    sf::Texture texture;
    bool textureLoaded;
};

}

#endif
