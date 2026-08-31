#include "Controller/PortalSystem.h"

#include "Controller/LevelGeometry.h"
#include "Model/Entity.h"
#include "Model/Level/Level.h"
#include "Model/Level/Pipe.h"
#include "Model/Player/Player.h"

#include <algorithm>
#include <cmath>

namespace controller {

void PortalSystem::clear() {
    inertPipeColumns.clear();
}

void PortalSystem::markInert(std::size_t column) {
    inertPipeColumns.push_back(column);
}

const model::Portal* PortalSystem::findEntryPortal(
    const model::Player& player,
    const model::Level& level,
    std::size_t currentArea,
    const std::vector<std::unique_ptr<model::Entity>>& entities) const {
    // A dying player never enters a pipe; which held direction counts depends on each
    // candidate pipe's own orientation, so that check moves inside the loop below.
    if (player.isDying()) {
        return nullptr;
    }
    for (const auto& e : entities) {
        auto* pipe = dynamic_cast<model::Pipe*>(e.get());
        if (!pipe || !pipe->isActive) continue;

        const model::Portal* portal = nullptr;
        for (const auto& p : level.portals(currentArea)) {
            if (p.sourceColumn == pipe->getSourceColumn()) {
                portal = &p;
                break;
            }
        }
        if (!portal) continue;

        // One-way pipes: the pipe the player arrived through is inert for this visit.
        if (std::find(inertPipeColumns.begin(), inertPipeColumns.end(),
                      pipe->getSourceColumn()) != inertPipeColumns.end()) {
            continue;
        }

        const model::Vector2& pPos = player.getPosition();
        const model::Vector2& pSize = player.getSize();
        const float buffer = 2.0f;

        if (pipe->getOrientation() == model::Pipe::Orientation::Horizontal) {
            // Touching it is the whole check — no held direction, no alignment window.
            // A horizontal mouth cannot afford the margins the cap below can: its 4x2
            // slab is only two tiles tall and its bottom edge normally sits flush on the
            // floor, so any "fit inside the mouth" test excludes super Mario outright
            // (32px of player in 32px of pipe, minus buffers) and clips small Mario by
            // the buffer as well. Since the pipe is solid, collision resolution has
            // already parked the player exactly against a face by the time this runs; an
            // overlap of the slightly inflated box is what "walked into it" means.
            const model::Vector2& qPos = pipe->getPosition();
            const model::Vector2& qSize = pipe->getSize();
            const bool touching =
                pPos.x + pSize.x >= qPos.x - buffer &&
                pPos.x <= qPos.x + qSize.x + buffer &&
                pPos.y + pSize.y >= qPos.y - buffer &&
                pPos.y <= qPos.y + qSize.y + buffer;
            if (touching) {
                return portal;
            }
            continue;
        }

        // Vertical: entry needs the player's feet resting on the cap and a real
        // footprint overlap with it (fully inside the pipe mouth).
        if (!player.getInputDown()) continue;
        const float feetY = pPos.y + pSize.y;
        const float onTop = std::abs(feetY - pipe->getPosition().y);
        const bool overlapsCap = pPos.x >= pipe->getPosition().x + buffer &&
                                 pPos.x + pSize.x <= pipe->getPosition().x + pipe->getSize().x - buffer;
        if (player.isGrounded && onTop < 4.0f && overlapsCap) {
            return portal;
        }
    }
    return nullptr;
}

float PortalSystem::landingY(
    const model::TileMap& map,
    const std::vector<std::unique_ptr<model::Entity>>& entities,
    std::size_t column,
    float playerHeight) const {
    const float groundTop = geometry::groundTopAt(map, column);
    float landY = groundTop - playerHeight;
    bool foundPipe = false;
    for (const auto& e : entities) {
        auto* pipe = dynamic_cast<model::Pipe*>(e.get());
        if (pipe && pipe->getSourceColumn() == column) {
            landY = pipe->getPosition().y - playerHeight;
            foundPipe = true;
            break;
        }
    }
    if (!foundPipe) {
        // If no Pipe entity exists (e.g. it is a destination-only pipe with no portal),
        // scan the TileMap in that column to find the top-most pipe tile.
        const std::size_t rows = map.getRows();
        for (int r = static_cast<int>(rows) - 1; r >= 0; --r) {
            char symbol = map.getTile(static_cast<std::size_t>(r), column);
            if (model::TileMap::isPipeSymbol(symbol)) {
                float pipeTop = static_cast<float>((rows - 1 - r) * model::TileMap::TileHeight);
                landY = pipeTop - playerHeight;
                break;
            }
        }
    }
    return landY;
}

}
