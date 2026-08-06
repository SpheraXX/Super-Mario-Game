#ifndef VIEW_BRICKBLOCKRENDERER_H
#define VIEW_BRICKBLOCKRENDERER_H

#include "View/Base/EntityRenderer.h"

#include <SFML/Graphics/Texture.hpp>

namespace model {
class BrickBlock;
}

namespace view {

// Draws a BrickBlock from the blocks tileset (the classic brick tile).
class BrickBlockRenderer : public TypedEntityRenderer<model::BrickBlock> {
public:
    BrickBlockRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::BrickBlock& brickBlock) const override;

private:
    sf::Texture texture;
    bool textureLoaded;
};

}

#endif
