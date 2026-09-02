#ifndef MODEL_TILEMAP_H
#define MODEL_TILEMAP_H

#include "Model/Core/Vector2.h"
#include "Model/World/WorldType.h"

#include <cstddef>
#include <string>
#include <vector>

namespace model {

// One enemy the level author placed, as read off the map. The map is the only source of
// initial enemy placement — nothing in the game code positions an enemy.
struct SpawnPoint {
    int id = 0;             // enemy id, matching EnemyFactory's enumeration
    std::size_t row = 0;    // grid row as stored (row 0 is the TOP line of the file)
    std::size_t column = 0;
};

class TileMap {
public:
    static constexpr std::size_t Rows = 16;
    static constexpr unsigned int TileWidth = 16;
    static constexpr unsigned int TileHeight = 16;

    // Background/decorative symbols: rendered like any other tile but passable
    // (excluded from tile collision).
    //
    // Bush and Hill are wider than the cell that carries them, and are the only tiles
    // anchored anywhere other than their own top-left corner: the author marks ONE cell
    // and the renderer paints the whole shape around it (see TileMapRenderer's
    // SceneryPart offsets). A bush is 3 cells wide centred on its marker; a hill is a
    // 5-3-1 pyramid whose marker is the middle of its BOTTOM row, so it is placed on the
    // ground row and grows upward. Kelp is an ordinary one-cell tile — a tall strand is
    // simply several Kelp cells stacked in the map file.
    static constexpr char CloudSymbol = 'O';
    static constexpr char SmallTreeSymbol = 'T';
    static constexpr char BushSymbol = 'w';
    static constexpr char HillSymbol = 'm';
    static constexpr char KelpSymbol = 'k';

    // Solid terrain that is not the ground strip: behaves exactly like an unbreakable
    // brick, so it is resolved by the tile pass rather than spawned as a Block entity.
    static constexpr char StairSymbol = 's';

    // A coin the author placed in the world, collected by walking through it. Spawns as a
    // MapCoin entity (see LevelScene::resetLevel), so it is never drawn as terrain and
    // never blocks movement. '$' rather than a letter on purpose: lowercase 'o' would be
    // far too easy to confuse with 'O' (the cloud) in a hand-written map.
    static constexpr char CoinSymbol = '$';

    // The end-of-level marker (see Model/Level/LevelGoal.h): touching it ends the run.
    // Author-placed, one per map (or per area, for an early exit) — there is no automatic
    // placement any more, so a map with none simply cannot be completed.
    static constexpr char GoalSymbol = 'E';

    // A moving platform (see Model/Level/Slider.h). A 2-cell run gives it its fixed shape
    // (its art is a constant 32x8, unlike a pipe's author-chosen height); the motion itself
    // — axis, travel distance, speed — comes from the map's '; slider=' header token, bound
    // to the run's leftmost column exactly as '; pipe=' binds to a Pipe.
    static constexpr char SliderSymbol = '=';

    // Castle furniture.
    //
    // ChainSymbol is the bridge deck Bowser stands on: ordinary SOLID terrain, so that
    // enemies collide with it too (the entity pass only resolves the PLAYER against solid
    // entities, so a bridge built out of entities would drop every enemy straight through
    // it). ChainTriggerSymbol is the axe at the far end — touching it erases every
    // ChainSymbol cell in the area at once, which is what drops the bridge and everything
    // riding it. See Model/Level/ChainTrigger.h.
    static constexpr char ChainSymbol = 'c';
    static constexpr char ChainTriggerSymbol = 'X';

    // Hazards that are spawned as entities from their marker cell and sweep through
    // terrain, so neither is solid and both are stripped to air at load like an enemy
    // digit. FirebarSymbol anchors a rotating LINE of flames (see FirebarBall.h);
    // LavaBubbleSymbol anchors one big fireball leaping in place (see LavaBubble.h).
    static constexpr char FirebarSymbol = 'r';
    static constexpr char LavaBubbleSymbol = 'b';

    // A Mushroom Retainer (Toad) standing where the author put him. Scenery with dialogue
    // rather than an obstacle: he is 16x24 and never blocks.
    static constexpr char RetainerSymbol = 'R';

    // Molten lava: purely decorative terrain (never solid — see isSolidTile), stacked by
    // the author the same way Kelp is. LavaTopSymbol is the wave-crest surface; LavaSymbol
    // is the plain fill beneath it, repeated for however many cells deep the pool is (a
    // 3-cell-deep pool is one LavaTopSymbol over two LavaSymbol).
    static constexpr char LavaTopSymbol = 'v';
    static constexpr char LavaSymbol = 'x';

    // The goal castle, as two multi-cell images rather than a grid of one-cell tiles.
    // It used to cost 21 symbols — most of the alphabet — because it was painted cell by
    // cell over a 5x5 silhouette. Nothing needed that granularity: the castle stopped
    // being the level's ending when LevelGoal ('E') took that job, so it is now pure
    // backdrop and can be drawn from the two rects the artwork is actually laid out as.
    //
    //   CastleUpperSymbol  tower, 3x2 cells (atlas 40,696 .. 87,727)
    //   CastleLowerSymbol  base,  5x3 cells (atlas 24,728 .. 103,775)
    //
    // Both are anchored at their own TOP-LEFT cell, the same as the cloud and the tree.
    // The tower is inset one cell from the base's left edge, so a whole castle is the
    // upper marker one column right of the lower marker and two screen-rows above it.
    //
    // Decorative, never solid (see isSolidTile): a 2-symbol castle has no way to express
    // per-cell collision, and the marker cell alone being solid would read as an
    // invisible wall. Mario walks in front of it, exactly as he does past a hill.
    static constexpr char CastleUpperSymbol = 'A';
    static constexpr char CastleLowerSymbol = 'H';

    // A horizontal pipe's lower body: one fixed 4x2-cell image (atlas 192,656 .. 255,687),
    // anchored at its own top-left cell like the cloud/tree/castle above. The vertical
    // riser some horizontal pipes wear is not part of this symbol at all -- it is built
    // from the ordinary 'P'/'Q'/'p'/'q' tiles, stacked above the lower part's right two
    // columns exactly like any other standing pipe, per the author's chosen height.
    //
    // Never solid via the tile pass: the artwork covers 7 cells the grid never actually
    // stores a symbol in (only the anchor does), so per-cell collision could only ever
    // block that one corner. Collision instead comes from a Pipe entity LevelScene always
    // spawns to cover the whole 4x2 footprint -- see Pipe::Orientation::Horizontal. A
    // '; pipe=' token binds a portal to it exactly as it binds one to a vertical pipe,
    // by matching the anchor's column; no new token syntax is needed.
    static constexpr char HorizontalPipeSymbol = 'F';

    static bool isCastleSymbol(char symbol);

    void loadFromFile(const std::string& filePath);

    // Load a pre-split grid (no header/metadata lines) of exactly Rows lines. Used by
    // Level to assemble the per-area grids it parsed out of a multi-area map file.
    void loadFromLines(const std::vector<std::string>& rows);

    // Enemy placements found in the map, in file order. The digits themselves are stripped
    // to empty tiles during load, so a spawn marker is never solid ground.
    const std::vector<SpawnPoint>& getSpawnPoints() const;

    // World-space top-left corner of a grid cell. Rows are stored top-down in the file,
    // so this is the one place that flip is written down.
    static Vector2 tileOrigin(std::size_t row, std::size_t column);

    // Append empty columns for the procedural level-completion zone (flagpole +
    // castle). Every new column mirrors the leftmost column's ground symbol ('G') so
    // the floor strip carries across the bonus area; everything else pads as air.
    void padRight(std::size_t extraColumns);

    // Rewrite one cell (used by the controller to paint the castle into the grid).
    void setTile(std::size_t row, std::size_t column, char symbol);

    char getTile(std::size_t row, std::size_t column) const;
    std::size_t getRows() const;
    std::size_t getColumns() const;

    // Optional metadata header (lines starting with ';' before the grid rows).
    const std::string& getLevelName() const;
    WorldType getWorldType() const;
    const std::string& getNextMapPath() const;
    bool hasNextMap() const;

    // Terrain a body can STAND ON: the ground strip, the stair block, the castle bridge
    // deck and a firebar's mount block.
    //
    // This exists because "solid" and "standable" are decided in four different places
    // that must agree, and did not: isSolidTile below (what blocks movement),
    // CollisionManager's isGroundTile (what the feet land on), geometry::isGroundSymbol
    // (where a warp drops you) and Enemy's ledge probe (what an enemy will walk onto).
    // A symbol added to the first but not the second is solid overhead and sideways yet
    // supports nothing, so bodies drop straight through the top of it — the bug the chain
    // shipped with, and the same one the stair block had before it. The three walkability
    // call sites now share this list and add only their own extras (the block ENTITIES
    // 'C'/'B'/'#', and pipes for the ones that want them), so new terrain is one edit.
    static bool isStandableTerrain(char symbol);

    static bool isSolidTile(char symbol);
    // True for the four pipe cells ('P'/'Q' mouth, 'p'/'q' shaft). Pipes are solid terrain
    // so that enemies collide with them, not just the player.
    static bool isPipeSymbol(char symbol);

private:
    void parseHeader(const std::string& line);

    std::vector<std::vector<char>> tiles;
    std::vector<SpawnPoint> spawnPoints;
    std::size_t columns = 0;  // map width in tiles, read from the file

    std::string levelName;
    WorldType worldType = WorldType::Overworld;
    std::string nextMapPath;
};

}

#endif
