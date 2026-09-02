#include "View/AssetManager.h"
#include "Model/Core/LogManager.h"

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
    if (uiFontLoaded) {
        for (unsigned int size = 8; size <= 56; size += 2) {
            const_cast<sf::Texture&>(uiFont.getTexture(size)).setSmooth(false);
        }
    }

    fallbackImage = sf::Image({1, 1}, sf::Color(255, 0, 255, 200));
    if (!fallbackTexture.loadFromImage(fallbackImage)) {
        model::LogManager::instance().warning("[AssetManager] Could not create fallback texture");
    }
}

// ── Font ──────────────────────────────────────────────────────────────────────

const sf::Font& AssetManager::getUiFont() const {
    return uiFont;
}

bool AssetManager::isFontLoaded() const {
    return uiFontLoaded;
}

// ── Images ────────────────────────────────────────────────────────────────────

const sf::Image& AssetManager::getImage(const std::string& filePath) {
    auto it = images.find(filePath);
    if (it != images.end()) {
        return it->second;
    }

    sf::Image img;
    if (!img.loadFromFile(filePath)) {
        model::LogManager::instance().error("[AssetManager] Failed to load texture: " + filePath);
        return fallbackImage;
    }

    auto [inserted, _] = images.emplace(filePath, std::move(img));
    return inserted->second;
}

// ── Textures ──────────────────────────────────────────────────────────────────

const sf::Texture& AssetManager::getTexture(const std::string& filePath, std::optional<sf::Color> colorKey) {
    std::string cacheKey = filePath;
    if (colorKey.has_value()) {
        cacheKey += "_#" + std::to_string(colorKey->toInteger());
    }

    auto it = textures.find(cacheKey);
    if (it != textures.end()) {
        return it->second.texture;
    }

    const sf::Image& srcImg = getImage(filePath);

    sf::Image img = srcImg;
    if (colorKey.has_value()) {
        img.createMaskFromColor(colorKey.value());
    }

    sf::Texture tex;
    if (!tex.loadFromImage(img)) {
        model::LogManager::instance().error("[AssetManager] Failed to upload texture: " + filePath);
        return fallbackTexture;
    }

    tex.setSmooth(isBackgroundAsset(filePath) && backgroundSmoothing);

    TextureData data;
    data.texture = std::move(tex);
    data.filePath = filePath;
    data.colorKey = colorKey;

    auto [inserted, _] = textures.emplace(cacheKey, std::move(data));
    return inserted->second.texture;
}

void AssetManager::clearUnused(const std::vector<std::string>& keepList) {
    auto it = textures.begin();
    while (it != textures.end()) {
        bool keep = false;
        for (const auto& keepPath : keepList) {
            if (it->first.find(keepPath) == 0) {
                keep = true;
                break;
            }
        }
        if (keep) {
            ++it;
        } else {
            it = textures.erase(it);
        }
    }

    auto imgIt = images.begin();
    while (imgIt != images.end()) {
        bool keep = std::find(keepList.begin(), keepList.end(), imgIt->first) != keepList.end();
        if (keep) {
            ++imgIt;
        } else {
            imgIt = images.erase(imgIt);
        }
    }
}

bool AssetManager::isBackgroundAsset(const std::string& filePath) {
    // Every full-screen backdrop in assets/images/ follows this naming convention
    // (bga_mainmenu.png, bga_options.png, bga_castle.jpg, worlds.json's per-world
    // "bga_image" entries, ...).
    return filePath.find("bga_") != std::string::npos;
}

void AssetManager::setBackgroundSmoothing(bool smooth) {
    backgroundSmoothing = smooth;
    for (auto& [key, data] : textures) {
        if (isBackgroundAsset(data.filePath)) {
            data.texture.setSmooth(smooth);
        }
    }
}

void AssetManager::reloadAll() {
    model::LogManager::instance().info("[AssetManager] Reloading all assets (Context Lost Recovery)...");

    if (!uiFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
        uiFontLoaded = false;
        model::LogManager::instance().error("[AssetManager] Failed to reload font");
    } else {
        uiFontLoaded = true;
        for (unsigned int size = 8; size <= 56; size += 2) {
            const_cast<sf::Texture&>(uiFont.getTexture(size)).setSmooth(false);
        }
    }

    for (auto& [key, data] : textures) {
        sf::Image img;
        if (img.loadFromFile(data.filePath)) {
            if (data.colorKey.has_value()) {
                img.createMaskFromColor(data.colorKey.value());
            }
            if (data.texture.loadFromImage(img)) {
                data.texture.setSmooth(isBackgroundAsset(data.filePath) && backgroundSmoothing);
            }
        }
    }

    for (auto& [path, img] : images) {
        if (!img.loadFromFile(path)) {
            model::LogManager::instance().warning("[AssetManager] Failed to reload optional asset: " + path);
        }
    }

    model::LogManager::instance().info("[AssetManager] Reload complete");
}

}  // namespace view
