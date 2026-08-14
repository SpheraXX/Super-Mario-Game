#include "Model/World/WorldTheme.h"

namespace model {

WorldTheme::WorldTheme(WorldType type, sf::Color backgroundColor, std::string tilesetPath,
             float gravityScale, float maxFallScale, float horizontalDrag,
             float deathBounceSpeed)
    : type(type),
      backgroundColor(backgroundColor),
      tilesetPath(std::move(tilesetPath)),
      gravityScale(gravityScale),
      maxFallScale(maxFallScale),
      horizontalDrag(horizontalDrag),
      deathBounceSpeed(deathBounceSpeed) {
}

WorldType WorldTheme::getType() const {
    return type;
}

const sf::Color& WorldTheme::getBackgroundColor() const {
    return backgroundColor;
}

const std::string& WorldTheme::getTilesetPath() const {
    return tilesetPath;
}

float WorldTheme::getGravityScale() const {
    return gravityScale;
}

float WorldTheme::getMaxFallScale() const {
    return maxFallScale;
}

float WorldTheme::getHorizontalDrag() const {
    return horizontalDrag;
}

float WorldTheme::getDeathBounceSpeed() const {
    return deathBounceSpeed;
}

}
