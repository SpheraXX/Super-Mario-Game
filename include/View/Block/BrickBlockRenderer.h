#ifndef VIEW_BRICKBLOCKRENDERER_H
#define VIEW_BRICKBLOCKRENDERER_H

#include "View/Base/EntityRenderer.h"

#include <SFML/Graphics/Texture.hpp>

namespace model {
class BrickBlock;
}

namespace view {

// Draws a BrickBlock, themed by the landscape it stands in: the brick behaves identically
// everywhere but is drawn from that landscape's quadrant of the shared sheet, so an
// underground brick is teal and a castle brick is grey. The world comes from the frame's
// RenderContext, so the block itself never needs to know where it is.
class BrickBlockRenderer : public TypedEntityRenderer<model::BrickBlock> {
public:
    BrickBlockRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::BrickBlock& brickBlock,
                     const RenderContext& ctx) const override;

private:
    const sf::Texture* texturePtr;
};

}

#endif
