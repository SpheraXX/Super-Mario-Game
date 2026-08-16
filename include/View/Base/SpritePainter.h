#ifndef VIEW_SPRITEPAINTER_H
#define VIEW_SPRITEPAINTER_H

#include "Model/Map/TileMap.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <string>

namespace view {

// SpritePainter is the single place that turns a source-frame + a target position into a
// drawn sprite on screen. It owns the whole life of one texture: loading the image,
// uploading it, masking out backdrop pixels, and drawing frames from it with snapped,
// integer-pixel positions (so tiles never smudge between cells). Everything that paints
// static artwork from a sheet — the tile map, pipes, the flagpole — goes through this
// facade instead of hand-rolling sprite setup, so texture loading and snapping behaviour
// live in exactly one spot.
class SpritePainter {
public:
    // Base atlas unit: tiles are drawn from 16x16 crops of the sheets and are scaled up
    // to one world tile (TileWidth/TileHeight) when drawn.
    static constexpr unsigned int SourceTileSize = 16;

    // The sheet stores its backdrop in several near-identical shades (e.g. 146,146,251 and
    // 148,148,255), so the color key matches within a tolerance instead of exactly —
    // otherwise stray backdrop pixels survive as a visible fringe. Safe because the closest
    // actual artwork color sits far outside this radius.
    static constexpr int ColorKeyTolerance = 16;

    SpritePainter() = default;
    explicit SpritePainter(const std::string& texturePath);

    // Loads the image and uploads it. Loading the same path twice replaces the contents.
    // Returns false (and leaves the painter empty) when the file cannot be read.
    bool load(const std::string& texturePath);
    bool isLoaded() const;

    // Masks every pixel of a rectangular source region whose color is within
    // ColorKeyTolerance of `transparentColor` to fully transparent. Pixels outside the
    // region are untouched, so different tiles can key different backdrop colors — which
    // is load-bearing, not a convenience: on the shared sheet the underwater hill's BODY
    // is painted the same (66,66,255) that is the kelp's backdrop, so keying that colour
    // across the whole image would erase the hill.
    //
    // This only edits the CPU-side image; nothing reaches the GPU until commitColorKeys().
    // Keying used to upload the whole image per call, which cost a full 2 MiB re-upload
    // for each of the ~15 keyed rects every time a level or area was (re)built.
    void applyColorKey(const sf::IntRect& area, sf::Color transparentColor);

    // Upload the accumulated colour-key edits. A no-op when nothing has changed, so it is
    // safe to call after every batch of registrations.
    void commitColorKeys();

    // Draws `frame` from the loaded sheet stretched by `scale`, snapped to integer
    // pixels. Does nothing while no image is loaded.
    void draw(sf::RenderTarget& window, const sf::IntRect& frame,
              const sf::Vector2f& position, const sf::Vector2f& scale) const;

    // Draws one SourceTileSize-square frame as exactly one world tile: uniform scale of
    // TileWidth/SourceTileSize, origin snapped to integer pixels.
    void drawCell(sf::RenderTarget& window, const sf::IntRect& frame,
                  const sf::Vector2f& origin) const;

private:
    sf::Image image;
    sf::Texture texture;
    bool loaded = false;
    bool colorKeyDirty = false;  // image has keyed edits not yet uploaded
};

}

#endif