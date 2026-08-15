#ifndef MODEL_WORLD_WORLDTHEME_H
#define MODEL_WORLD_WORLDTHEME_H

#include "Model/World/WorldType.h"

#include <SFML/Graphics/Color.hpp>

#include <string>

namespace model {

// A playable world: the graphics theme (background colour + tileset) together with the
// physics tuning applied to every character that spawns into it. Worlds are immutable
// descriptors built once by the WorldSet registry; the gameplay reads them, never mutates.
class WorldTheme {
public:
    WorldTheme(WorldType type, sf::Color backgroundColor, std::string tilesetPath,
          float gravityScale, float maxFallScale, float horizontalDrag,
          float deathBounceSpeed);

    WorldType getType() const;
    const sf::Color& getBackgroundColor() const;
    const std::string& getTilesetPath() const;

    // Physics multipliers: 1.0 keeps the land (Overworld) constants untouched, so the
    // tuned jump arcs are exactly preserved outside of special worlds.
    float getGravityScale() const;
    float getMaxFallScale() const;
    // Extra horizontal resistance per second (0 = none). Underwater drag keeps motion
    // floaty and makes the player settle instead of skating around.
    float getHorizontalDrag() const;
    // Upward launch of a dying body (units/s). Overworld and Castle use the base -200;
    // Underwater's much smaller pop keeps the slow death fall from hovering.
    float getDeathBounceSpeed() const;
    // Upward launch of a coin popped out of a bumped block (units/s). The coin always
    // rises 1.5x as fast as the world's death bounce, so the ratio is structural: land
    // and Castle pop exactly -300, underwater -135, and retuning a world's death bounce
    // automatically retunes its coin pop.
    float getCoinPopSpeed() const;

private:
    WorldType type;
    sf::Color backgroundColor;
    std::string tilesetPath;
    float gravityScale;
    float maxFallScale;
    float horizontalDrag;
    float deathBounceSpeed;
};

}

#endif
