#ifndef VIEW_ASSETMANAGER_H
#define VIEW_ASSETMANAGER_H

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace view {

// Singleton owning all shared assets (font, textures). Uses Lazy-Loading + Caching:
// a texture is loaded from disk only the first time it is requested, then kept alive
// for the lifetime of the process. Every renderer gets the same sf::Texture reference,
// so each image file occupies RAM exactly once regardless of how many UI elements
// or sprites reference it.
//
// Why unordered_map<string, Texture> instead of a flat list:
//   Keying by file path lets callers request any texture by name without the manager
//   knowing the full catalogue in advance (Open-Closed: add a new asset folder without
//   touching this class). Lookup is O(1) amortised.
class AssetManager {
public:
    static AssetManager& instance();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // ── Font ─────────────────────────────────────────────────────────────────
    // The SMB-style pixel font (Press Start 2P). isFontLoaded() reports whether
    // the file was actually found; callers fall back to not drawing text if not.
    const sf::Font& getUiFont() const;
    bool isFontLoaded() const;

    // ── Images (Raw pixel data) ──────────────────────────────────────────────
    // Returns a const reference to the cached raw image for 'filePath'.
    // Use this when you need pixel-level manipulation (e.g. SpritePainter)
    // rather than direct rendering.
    const sf::Image& getImage(const std::string& filePath);

    // ── Textures ──────────────────────────────────────────────────────────────
    // Returns a const reference to the cached texture for 'filePath'.
    // If 'colorKey' is provided, the image is loaded, masked with that color,
    // and then uploaded to VRAM.
    // Loads from disk on the first call for that path/color combo (Lazy Loading).
    // Returns a 1×1 transparent fallback texture on load failure so callers
    // never receive a dangling reference.
    const sf::Texture& getTexture(const std::string& filePath, std::optional<sf::Color> colorKey = std::nullopt);

    // Removes all cached textures whose keys are NOT in 'keepList'.
    // Call this when transitioning between worlds to free VRAM for the next set.
    // Always keeps the UI/font atlas alive; it is never included in eviction.
    void clearUnused(const std::vector<std::string>& keepList);

    // Reloads all currently cached images and textures from disk.
    // Essential for recovering from Graphics Context Loss (e.g., toggling Fullscreen).
    void reloadAll();

private:
    AssetManager();

    sf::Font uiFont;
    bool uiFontLoaded;

    // Cache structure for textures to support color keying and reloading
    struct TextureData {
        sf::Texture texture;
        std::string filePath;
        std::optional<sf::Color> colorKey;
    };

    std::unordered_map<std::string, sf::Image> images;
    std::unordered_map<std::string, TextureData> textures;

    // A 1×1 transparent texture/image returned whenever a file cannot be loaded.
    sf::Image fallbackImage;
    sf::Texture fallbackTexture;
};

}  // namespace view

#endif
