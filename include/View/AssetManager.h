#ifndef VIEW_ASSETMANAGER_H
#define VIEW_ASSETMANAGER_H

#include <SFML/Graphics/Font.hpp>

namespace view {

// Singleton owning the assets shared by every screen (the UI font today, textures and
// sounds later). Loading once here keeps memory low and gives every renderer exactly
// the same instance, so the HUD, menus and overlays always agree on the look.
class AssetManager {
public:
    static AssetManager& instance();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // The SMB-style pixel font (Press Start 2P). isFontLoaded() reports whether the
    // file was actually found; callers fall back to not drawing text if it wasn't.
    const sf::Font& getUiFont() const;
    bool isFontLoaded() const;

private:
    AssetManager();

    sf::Font uiFont;
    bool uiFontLoaded;
};

}

#endif
