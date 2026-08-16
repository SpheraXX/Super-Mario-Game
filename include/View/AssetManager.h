#ifndef VIEW_ASSETMANAGER_H
#define VIEW_ASSETMANAGER_H

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <string>
#include <unordered_map>

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

    // ── Textures ──────────────────────────────────────────────────────────────
    // Returns a const reference to the cached texture for 'filePath'.
    // Loads from disk on the first call for that path (Lazy Loading).
    // Returns a 1×1 transparent fallback texture on load failure so callers
    // never receive a dangling reference.
    //
    // Usage:  const sf::Texture& tex = AssetManager::instance().getTexture("assets/ui/button.png");
    const sf::Texture& getTexture(const std::string& filePath);

    // Removes all cached textures whose keys are NOT in 'keepList'.
    // Call this when transitioning between worlds to free VRAM for the next set.
    // Always keeps the UI/font atlas alive; it is never included in eviction.
    void clearUnused(const std::vector<std::string>& keepList);

private:
    AssetManager();

    sf::Font uiFont;
    bool uiFontLoaded;

    std::unordered_map<std::string, sf::Texture> textures;

    // A 1×1 transparent texture returned whenever a file cannot be loaded.
    sf::Texture fallbackTexture;
};

}  // namespace view

#endif
