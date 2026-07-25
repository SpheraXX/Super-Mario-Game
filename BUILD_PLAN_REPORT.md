# Build Plan Report

## What We Have Built

We now have a very small Mario-style foundation that can load and draw a tile map from file.

- The map is `16 x 32` tiles in data.
- The visible window uses rectangular tiles with a wider-than-tall shape.
- Empty tiles are skipped, so the map file only needs to describe the interesting cells.
- A brick tile and a coin block tile are both supported through a tileset map.

## Current Game Slice

- `#` is used for the brick block.
- `C` is used for a simple coin block.
- `.` means empty space.
- The map loader reads plain text files from `assets/maps/`.
- The renderer draws the correct tile from `assets/blocks.png` by looking up each map symbol.

## Structure Design

The project is organized in a simple MVC-style layout:

- `include/Model/` and `src/Model/` hold game data and basic entity classes.
- `include/View/` and `src/View/` hold rendering code.
- `src/main.cpp` starts the window, loads the map, and drives the draw loop.

The model side is intentionally small for now:

- `Entity` is the base class for future game objects.
- `Block` extends `Entity` for static level objects.
- `CoinBlock` extends `Block` and can later gain behavior such as spawning a coin.

## Why This Shape

This structure keeps the first version simple while leaving room for later work.

- New block types can be added by registering more tile symbols.
- New gameplay objects can inherit from `Entity` without changing the renderer.
- Level data stays in files, so maps can be changed without editing code.

## Next Step

The next natural step is to define more block symbols and link them to tile regions in the tileset.
