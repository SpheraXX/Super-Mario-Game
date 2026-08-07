#include "Model/World/World.h"

namespace model {

World::World(WorldType type, sf::Color backgroundColor, std::string tilesetPath,
             float gravityScale, float maxFallScale, float horizontalDrag)
    : type(type),
      backgroundColor(backgroundColor),
      tilesetPath(std::move(tilesetPath)),
      gravityScale(gravityScale),
      maxFallScale(maxFallScale),
      horizontalDrag(horizontalDrag) {
}

WorldType World::getType() const {
    return type;
}

const sf::Color& World::getBackgroundColor() const {
    return backgroundColor;
}

const std::string& World::getTilesetPath() const {
    return tilesetPath;
}

float World::getGravityScale() const {
    return gravityScale;
}

float World::getMaxFallScale() const {
    return maxFallScale;
}

float World::getHorizontalDrag() const {
    return horizontalDrag;
}

}
