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
    // Entry needs the player to hold Down and not be dying; the rest is geometry.
    if (!player.getInputDown() || player.isDying()) {
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
        const float feetY = pPos.y + player.getSize().y;
        const float onTop = std::abs(feetY - pipe->getPosition().y);
        // Entry needs the player's feet resting on the cap and a real footprint
        // overlap with it (slightly forgiving at the very edge).
        const bool overlapsCap = pPos.x + player.getSize().x > pipe->getPosition().x + 1.0f &&
                                 pPos.x < pipe->getPosition().x + pipe->getSize().x - 1.0f;
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
    for (const auto& e : entities) {
        auto* pipe = dynamic_cast<model::Pipe*>(e.get());
        if (pipe && pipe->getSourceColumn() == column) {
            landY = pipe->getPosition().y - playerHeight;
            break;
        }
    }
    return landY;
}

}
