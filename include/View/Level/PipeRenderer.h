#ifndef VIEW_LEVEL_PIPERENDERER_H
#define VIEW_LEVEL_PIPERENDERER_H

#include "View/Base/EntityRenderer.h"

#include <SFML/Graphics/Texture.hpp>

namespace model {
class Pipe;
}

namespace view {

// Draws a Pipe from super_mario_asset.png: the entrance cap on the top cell, the plain
// body on every cell below it (all 16x16 source tiles, one per world cell).
class PipeRenderer : public TypedEntityRenderer<model::Pipe> {
public:
    PipeRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::Pipe& pipe,
                     const RenderContext& ctx) const override;

private:
    sf::Texture texture;
    bool textureLoaded;
};

}

#endif