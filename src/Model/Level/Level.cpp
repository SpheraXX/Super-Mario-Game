#include "Model/Level/Level.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>

namespace model {

namespace {
std::string trim(const std::string& value) {
    const std::size_t begin = value.find_first_not_of(" \t");
    if (begin == std::string::npos) {
        return std::string();
    }
    const std::size_t end = value.find_last_not_of(" \t");
    return value.substr(begin, end - begin + 1);
}

std::size_t parseSizeT(const std::string& text) {
    return static_cast<std::size_t>(std::strtoull(text.c_str(), nullptr, 10));
}
}

void Level::loadFromFile(const std::string& filePath) {
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Could not open level file: " + filePath);
    }

    areas.clear();
    levelName.clear();
    nextMapPath.clear();

    std::vector<std::string> gridRows;
    auto commitArea = [&]() {
        if (!gridRows.empty()) {
            areas.back().map.loadFromLines(gridRows);
            gridRows.clear();
        }
    };

    std::string line;
    while (std::getline(input, line)) {
        // Blank lines separate segments; skip them.
        if (line.size() == 0 || line[0] == '\r') {
            continue;
        }

        if (line[0] == ';') {
            const std::size_t eq = line.find('=');
            if (eq == std::string::npos) {
                // Bare markers with no value: `; area` starts a new segment.
                if (trim(line.substr(1)) == "area") {
                    commitArea();
                    areas.push_back(Area{});
                }
                continue;
            }
            const std::string key = trim(line.substr(1, eq - 1));
            const std::string value = trim(line.substr(eq + 1));

            if (key == "area") {
                commitArea();
                areas.push_back(Area{});
            } else if (key == "name" || key == "level") {
                levelName = value;
            } else if (key == "next") {
                nextMapPath = value;
            } else if (key == "world") {
                // A world declared before the first `; area` implicitly opens area 0
                // (legacy single-area files). Set the CURRENT pending area's world.
                if (areas.empty()) {
                    areas.push_back(Area{});
                }
                areas.back().world = worldTypeFromString(value);
} else if (key == "pipe") {
                if (areas.empty()) {
                    areas.push_back(Area{});
                }
                Portal portal{};
                std::size_t cursor = 0;
                while (cursor <= value.size()) {
                    const std::size_t comma = value.find(',', cursor);
                    const std::string token =
                        value.substr(cursor, comma == std::string::npos ? std::string::npos
                                                                        : comma - cursor);
                    const std::size_t colon = token.find(':');
                    if (colon != std::string::npos && colon + 1 < token.size()) {
                        const std::string k = trim(token.substr(0, colon));
                        const std::string v = trim(token.substr(colon + 1));
                        if (k == "col") {
                            portal.sourceColumn = parseSizeT(v);
                        } else if (k == "enter") {
                            portal.direction =
                                (v == "up" || v == "Up") ? PortalDirection::Up
                                                         : PortalDirection::Down;
                        } else if (k == "to" || k == "dest") {
                            // "to:<area>:<column>".
                            const std::size_t sep = v.find(':');
                            if (sep != std::string::npos) {
                                portal.destinationArea = parseSizeT(v.substr(0, sep));
                                portal.destinationColumn = parseSizeT(v.substr(sep + 1));
                            }
                        }
                    }
                    if (comma == std::string::npos) {
                        break;
                    }
                    cursor = comma + 1;
                }
                areas.back().portals.push_back(portal);
            }
            continue;
        }

        // A grid row belongs to the current pending area (created implicitly if needed).
        if (areas.empty()) {
            areas.push_back(Area{});
        }
        gridRows.push_back(line);
        if (gridRows.size() == TileMap::Rows) {
            commitArea();
        }
    }
    commitArea();

    if (areas.size() == 0) {
        throw std::runtime_error("Level file contains no playable area: " + filePath);
    }

    // Legacy single-area files keep any world (a lone underwater/castle map played exactly
    // like before). Once a file declares MULTIPLE areas the terminal world must be the
    // Overworld: the procedural completion zone (flagpole + castle) only reads as a goal
    // there, and every non-final area has to be exited through a pipe.
    if (areas.size() > 1 && areas.back().world != WorldType::Overworld) {
        throw std::runtime_error("Last area of a multi-area level must be an Overworld: " + filePath);
    }
}

std::size_t Level::areaCount() const {
    return areas.size();
}

TileMap& Level::areaMap(std::size_t index) {
    return areas.at(index).map;
}

const TileMap& Level::areaMap(std::size_t index) const {
    return areas.at(index).map;
}

WorldType Level::areaWorld(std::size_t index) const {
    return areas.at(index).world;
}

const std::vector<Portal>& Level::portals(std::size_t index) const {
    return areas.at(index).portals;
}

const std::string& Level::getLevelName() const {
    return levelName;
}

const std::string& Level::getNextMapPath() const {
    return nextMapPath;
}

bool Level::hasNextMap() const {
    return !nextMapPath.empty();
}

}