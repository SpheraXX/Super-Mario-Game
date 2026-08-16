#include "View/Level/SliderRenderer.h"

#include "Model/Level/Slider.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>

namespace view {

namespace {
// Atlas coordinates in items.png: a repeating plank/conveyor pattern, already carrying
// real alpha (the small gaps at the crest of each peak are pre-keyed transparent in the
// file), so no colour-key masking is needed at load time.
constexpr int ArtX = 26;
constexpr int ArtY = 38;
constexpr int ArtWidth = 32;
constexpr int ArtHeight = 8;
}

SliderRenderer::SliderRenderer()
    : painter("assets/items.png") {
}

void SliderRenderer::renderTyped(sf::RenderTarget& window, const model::Slider& slider,
                                 const RenderContext& /* ctx */) const {
    if (!painter.isLoaded()) {
        return;
    }

    painter.draw(window, {{ArtX, ArtY}, {ArtWidth, ArtHeight}},
                 {std::round(slider.getPosition().x), std::round(slider.getPosition().y)},
                 {1.0f, 1.0f});
}

}
