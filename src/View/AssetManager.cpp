#include "View/AssetManager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <iostream>

namespace view {

AssetManager &AssetManager::instance() {
  static AssetManager singleton;
  return singleton;
}

AssetManager::AssetManager()
    : uiFontLoaded(
          uiFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
  // Press Start 2P is a pixel font: its glyph pages must be sampled without
  // bilinear filtering, or the thin strokes smear once the offscreen frame is
  // upscaled to the window.
  if (uiFontLoaded) {
    for (unsigned int size = 8; size <= 56; size += 2) {
      const_cast<sf::Texture &>(uiFont.getTexture(size)).setSmooth(false);
    }
  }

  // The fallback texture is a single opaque magenta pixel — it is visually
  // obvious that something went wrong, which is better than silently invisible.
  fallbackImage = sf::Image({1, 1}, sf::Color(255, 0, 255, 200));
  if (!fallbackTexture.loadFromImage(fallbackImage)) {
    // If even this fails, the Texture stays default-constructed (white pixel).
    std::cerr << "[AssetManager] WARNING: could not create fallback texture\n";
  }
}

// ── Font
// ──────────────────────────────────────────────────────────────────────

const sf::Font &AssetManager::getUiFont() const { return uiFont; }

bool AssetManager::isFontLoaded() const { return uiFontLoaded; }

// ── Images
// ────────────────────────────────────────────────────────────────────

const sf::Image &AssetManager::getImage(const std::string &filePath) {
  auto it = images.find(filePath);
  if (it != images.end()) {
    return it->second;
  }

  sf::Image img;
  if (!img.loadFromFile(filePath)) {
    std::cerr << "[AssetManager] ERROR: failed to load image: " << filePath
              << "\n";
    return fallbackImage;
  }

  auto [inserted, _] = images.emplace(filePath, std::move(img));
  return inserted->second;
}

// ── Textures
// ──────────────────────────────────────────────────────────────────

const sf::Texture &AssetManager::getTexture(const std::string &filePath,
                                            std::optional<sf::Color> colorKey) {
  // Generate cache key
  std::string cacheKey = filePath;
  if (colorKey.has_value()) {
    cacheKey += "_#" + std::to_string(colorKey->toInteger());
  }

  // Cache hit — return immediately, no disk I/O.
  auto it = textures.find(cacheKey);
  if (it != textures.end()) {
    return it->second.texture;
  }

  // Cache miss — reuse getImage() to avoid reading from disk a second time
  // when both getImage and getTexture are called for the same file.
  const sf::Image &srcImg = getImage(filePath);

  sf::Image img = srcImg; // local mutable copy for optional color masking
  if (colorKey.has_value()) {
    img.createMaskFromColor(colorKey.value());
  }

  // Upload to VRAM
  sf::Texture tex;
  if (!tex.loadFromImage(img)) {
    std::cerr << "[AssetManager] ERROR: failed to upload texture: " << filePath
              << "\n";
    return fallbackTexture;
  }

  // Pixel art textures should never be smoothed: bilinear filtering blurs
  // the hard edges that define the 8-bit aesthetic.
  tex.setSmooth(false);

  TextureData data;
  data.texture = std::move(tex);
  data.filePath = filePath;
  data.colorKey = colorKey;

  auto [inserted, _] = textures.emplace(cacheKey, std::move(data));
  return inserted->second.texture;
}

void AssetManager::clearUnused(const std::vector<std::string> &keepList) {
  auto it = textures.begin();
  while (it != textures.end()) {
    bool keep = false;
    for (const auto &keepPath : keepList) {
      // Keep if the cache key starts with keepPath
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
    bool keep = std::find(keepList.begin(), keepList.end(), imgIt->first) !=
                keepList.end();
    if (keep) {
      ++imgIt;
    } else {
      imgIt = images.erase(imgIt);
    }
  }
}

void AssetManager::reloadAll() {
  std::cout
      << "[AssetManager] Reloading all assets (Context Lost Recovery)...\n";

  if (!uiFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
    uiFontLoaded = false;
    std::cerr << "[AssetManager] ERROR: failed to reload font.\n";
  } else {
    uiFontLoaded = true;
    for (unsigned int size = 8; size <= 56; size += 2) {
      const_cast<sf::Texture &>(uiFont.getTexture(size)).setSmooth(false);
    }
  }

  for (auto &[key, data] : textures) {
    sf::Image img;
    if (img.loadFromFile(data.filePath)) {
      if (data.colorKey.has_value()) {
        img.createMaskFromColor(data.colorKey.value());
      }
      if (data.texture.loadFromImage(img)) {
        data.texture.setSmooth(false);
      }
    }
  }

  for (auto &[path, img] : images) {
    if (!img.loadFromFile(path)) {
      std::cerr << "[AssetManager] WARNING: failed to reload image: " << path
                << "\n";
    }
  }

  std::cout << "[AssetManager] Reload complete.\n";
}

} // namespace view
