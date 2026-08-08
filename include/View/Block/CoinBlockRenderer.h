#ifndef VIEW_COINBLOCKRENDERER_H
#define VIEW_COINBLOCKRENDERER_H

#include "View/Base/EntityRenderer.h"

#include <SFML/Graphics/Texture.hpp>

namespace model {
class CoinBlock;
}

namespace view {

// Draws a CoinBlock from the blocks tileset: the ? block while it still holds a coin,
// the plain used block afterwards. While the collected coin is popping out, the coin
// sprite (from super_mario_asset.png) is drawn rising above the block.
class CoinBlockRenderer : public TypedEntityRenderer<model::CoinBlock> {
public:
    CoinBlockRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::CoinBlock& coinBlock,
                     const RenderContext& ctx) const override;

private:
    sf::Texture texture;
    bool textureLoaded;
    sf::Texture coinTexture;
    bool coinTextureLoaded;
};

}

#endif
