#include "View/AssetManager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <iostream>

namespace view {

AssetManager& AssetManager::instance() {
    static AssetManager singleton;
    return singleton;
}

AssetManager::AssetManager()
    : uiFontLoaded(uiFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
    // Press Start 2P is a pixel font: its glyph pages must be sampled without
    // bilinear filtering, or the thin strokes smear once the offscreen frame is
    // upscaled to the window.
    if (uiFontLoaded) {
        for (unsigned int size = 8; size <= 56; size += 2) {
            const_cast<sf::Texture&>(uiFont.getTexture(size)).setSmooth(false);
        }
    }

    // The fallback texture is a single opaque magenta pixel — it is visually
    // obvious that something went wrong, which is better than silently invisible.
    sf::Image fallbackImage({1, 1}, sf::Color(255, 0, 255, 200));
    if (!fallbackTexture.loadFromImage(fallbackImage)) {
        // If even this fails, the Texture stays default-constructed (white pixel).
        std::cerr << "[AssetManager] WARNING: could not create fallback texture\n";
    }
}

// ── Font ──────────────────────────────────────────────────────────────────────

const sf::Font& AssetManager::getUiFont() const {
    return uiFont;
}

bool AssetManager::isFontLoaded() const {
    return uiFontLoaded;
}

// ── Textures ──────────────────────────────────────────────────────────────────

const sf::Texture& AssetManager::getTexture(const std::string& filePath) {
    // Cache hit — return immediately, no disk I/O.
    auto it = textures.find(filePath);
    if (it != textures.end()) {
        return it->second;
    }

    // Cache miss — load from disk and insert.
    sf::Texture tex;
    if (!tex.loadFromFile(filePath)) {
        std::cerr << "[AssetManager] ERROR: failed to load texture: " << filePath << "\n";
        return fallbackTexture;
    }

    // Pixel art textures should never be smoothed: bilinear filtering blurs
    // the hard edges that define the 8-bit aesthetic.
    tex.setSmooth(false);

    auto [inserted, _] = textures.emplace(filePath, std::move(tex));
    return inserted->second;
}

void AssetManager::clearUnused(const std::vector<std::string>& keepList) {
    auto it = textures.begin();
    while (it != textures.end()) {
        bool keep = std::find(keepList.begin(), keepList.end(), it->first) != keepList.end();
        if (keep) {
            ++it;
        } else {
            it = textures.erase(it);
        }
    }
}

}  // namespace view
