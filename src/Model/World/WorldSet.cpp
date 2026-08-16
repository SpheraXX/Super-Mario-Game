#include "Model/World/WorldSet.h"

#include <map>

namespace model {

const WorldTheme& WorldSet::forType(WorldType type) {
    static const std::map<WorldType, WorldTheme> worlds{
        {WorldType::Overworld,
         // No horizontal drag on land: the player's own friction and acceleration already
         // shape the ground feel, and stacking a per-world bleed on top of them was what
         // made a released key coast. Drag stays for water, where floatiness is the point.
         WorldTheme(WorldType::Overworld, sf::Color(92, 148, 252), "assets/blocks.png",
               1.0f, 1.0f, 0.0f, -200.0f)},
        // Simplified underwater: weaker gravity, a slower fall ceiling and horizontal
        // drag; the player swims with the held jump key (see Player::handleInput). The
        // death pop is tiny: with 0.35 gravity a -200 launch would hover for 4+ tiles.
        {WorldType::Underwater,
         WorldTheme(WorldType::Underwater, sf::Color(16, 64, 160), "assets/blocks.png",
               0.35f, 0.45f, 1.2f, -90.0f)},
        // Castle: land physics under a dark sky, with a gray ground theme.
        {WorldType::Castle,
         WorldTheme(WorldType::Castle, sf::Color(24, 24, 40), "assets/blocks.png",
               1.0f, 1.0f, 0.0f, -200.0f)},
    };

    const auto found = worlds.find(type);
    return (found != worlds.end()) ? found->second : worlds.at(WorldType::Overworld);
}

}
