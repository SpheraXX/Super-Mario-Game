#ifndef VIEW_MAPCOINRENDERER_H
#define VIEW_MAPCOINRENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

namespace model {
class MapCoin;
}

namespace view {

// Draws a placed coin, cycling atlas::CoinSpin off the coin's own animation clock. It gets
// its own renderer rather than reusing ItemFrameRenderer because that template draws a
// single fixed frame; the phase lives on the model, so this stays stateless.
class MapCoinRenderer : public SpriteEntityRenderer<model::MapCoin> {
public:
    MapCoinRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::MapCoin& coin,
                     const RenderContext& ctx) const override;
};

}

#endif
