#include "Model/World/WorldSet.h"

#include <map>

namespace model {

const WorldTheme& WorldSet::forType(WorldType type) {
    static const std::map<WorldType, WorldTheme> worlds{
        {WorldType::Overworld,
         WorldTheme(WorldType::Overworld, sf::Color(92, 148, 252), "assets/blocks.png",
               1.0f, 1.0f, 0.4f)},
        // Simplified underwater: weaker gravity, a slower fall ceiling and horizontal
        // drag; the player swims with the held jump key (see Player::handleInput).
        {WorldType::Underwater,
         WorldTheme(WorldType::Underwater, sf::Color(16, 64, 160), "assets/blocks.png",
               0.35f, 0.45f, 1.2f)},
        // Castle: land physics under a dark sky, with a gray ground theme.
        {WorldType::Castle,
         WorldTheme(WorldType::Castle, sf::Color(24, 24, 40), "assets/blocks.png",
               1.0f, 1.0f, 0.0f)},
    };

    const auto found = worlds.find(type);
    return (found != worlds.end()) ? found->second : worlds.at(WorldType::Overworld);
}

}
