#include "View/AssetManager.h"

namespace view {

AssetManager& AssetManager::instance() {
    static AssetManager singleton;
    return singleton;
}

AssetManager::AssetManager()
    : uiFontLoaded(uiFont.openFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
}

const sf::Font& AssetManager::getUiFont() const {
    return uiFont;
}

bool AssetManager::isFontLoaded() const {
    return uiFontLoaded;
}

}
