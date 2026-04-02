//
// Created by Daniel White on 3/6/26.
//

#ifndef ASTGEN_TILETYPES_H
#define ASTGEN_TILETYPES_H

enum class TileType {
    Empty = 0,
    Dust,
    Rock,
    DenseRock,
    SuperDenseRock
};

enum class OreType {
    EmptyOre = 0,
    DustOre,
    RockOre,
    DenseRockOre,
    SuperDenseRockOre
};

using TileGeometry = uint8_t;

class Tile {
public:
    TileType tileType = TileType::Empty;
    OreType oreType = OreType::EmptyOre;
    TileGeometry geometry = 0;
};

#endif //ASTGEN_TILETYPES_H