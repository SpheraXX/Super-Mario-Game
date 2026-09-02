#include "Model/Editor/EditableMap.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

namespace model {

namespace {
constexpr char EmptySymbol = '.';
constexpr char GroundSymbol = 'G';
}

EditableMap::EditableMap(WorldType world) : worldType(world) {
    tiles.assign(Rows, std::vector<char>(m_columns, EmptySymbol));
    // Rows 0 and 1 are the bottom two rows (file lines 1-2): the default floor a new
    // map starts with, so a fresh grid isn't an instant fall into the void.
    for (std::size_t row = 0; row < 2 && row < Rows; ++row) {
        std::fill(tiles[row].begin(), tiles[row].end(), GroundSymbol);
    }
}

char EditableMap::getTile(std::size_t row, std::size_t column) const {
    return tiles.at(row).at(column);
}

void EditableMap::setTile(std::size_t row, std::size_t column, char symbol) {
    tiles.at(row).at(column) = symbol;
}

void EditableMap::placePlayerSpawn(std::size_t row, std::size_t column) {
    for (std::size_t r = 0; r < Rows; ++r) {
        for (std::size_t c = 0; c < m_columns; ++c) {
            if (tiles[r][c] == 'M') {
                tiles[r][c] = EmptySymbol;
            }
        }
    }
    setTile(row, column, 'M');
}

EditableMap::ValidationResult EditableMap::validate() const {
    int spawnCount = 0;
    int goalCount = 0;
    for (const auto& row : tiles) {
        for (char symbol : row) {
            if (symbol == 'M') ++spawnCount;
            if (symbol == 'E') ++goalCount;
        }
    }

    if (spawnCount == 0) {
        return {false, "Place a player start (M) before saving."};
    }
    if (goalCount == 0) {
        return {false, "Place a goal (E) before saving."};
    }
    if (goalCount > 1) {
        return {false, "Only one goal (E) is allowed."};
    }
    return {true, ""};
}

TileMap EditableMap::toTileMap() const {
    std::vector<std::string> rows;
    rows.reserve(Rows);
    for (const auto& row : tiles) {
        rows.emplace_back(row.begin(), row.end());
    }
    TileMap map;
    map.loadFromLines(rows);
    return map;
}

std::string EditableMap::serialize() const {
    std::string out;
    out += "; name=" + levelName + "\n";
    out += "; world=" + worldTypeToString(worldType) + "\n";
    for (const auto& row : tiles) {
        out.append(row.begin(), row.end());
        out += "\n";
    }
    return out;
}

std::string EditableMap::sanitizeFileName(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out += c;
        } else if (c == ' ' || c == '_' || c == '-') {
            out += '_';
        }
    }
    if (out.empty()) {
        out = "custom_map";
    }
    return out;
}

bool EditableMap::saveToFile(const std::string& dir) const {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        return false;
    }

    const std::string path = dir + sanitizeFileName(levelName) + ".map";
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << serialize();
    return true;
}

std::vector<std::string> EditableMap::listCustomMaps(const std::string& dir) {
    std::vector<std::string> result;
    std::error_code ec;
    if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec)) {
        return result;
    }

    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (entry.is_regular_file() && entry.path().extension() == ".map") {
            result.push_back(entry.path().string());
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

} // namespace model
