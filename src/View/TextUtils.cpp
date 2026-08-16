#include "View/TextUtils.h"

#include <SFML/Graphics/Text.hpp>

namespace view {
namespace text {

unsigned int fitCharacterSize(const sf::Font& font, const std::string& content,
                              float maxWidth, unsigned int preferredSize) {
    for (unsigned int size = preferredSize; size >= 8; --size) {
        const sf::Text probe(font, content, size);
        if (probe.getLocalBounds().size.x <= maxWidth) {
            return size;
        }
    }
    return 8;
}

}
}
