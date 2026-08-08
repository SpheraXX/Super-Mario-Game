#include "View/AssetManager.h"

#include <SFML/Graphics/Texture.hpp>

namespace view {

AssetManager& AssetManager::instance() {
    static AssetManager singleton;
    return singleton;
}

AssetManager::AssetManager()
    : uiFontLoaded(uiFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
    // Press Start 2P is a pixel font: its glyph pages must be sampled without bilinear
    // filtering, or the thin strokes smear once the offscreen frame is upscaled to the
    // window. SFML exposes the pages as const (they are lazily created buffers), so the
    // standard pixel-font workaround is a const_cast to flip the smooth flag.
    if (uiFontLoaded) {
        for (unsigned int size = 8; size <= 56; size += 2) {
            const_cast<sf::Texture&>(uiFont.getTexture(size)).setSmooth(false);
        }
    }
}

const sf::Font& AssetManager::getUiFont() const {
    return uiFont;
}

bool AssetManager::isFontLoaded() const {
    return uiFontLoaded;
}

}
