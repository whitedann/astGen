//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_CHUNKENUMS_H
#define ASTGEN_CHUNKENUMS_H

static constexpr int CHUNK_SIZE_PX = 32;
static constexpr int TILE_SIZE_PX = 1;

enum class ChunkState {
    LOADING,
    LOADED,
    UNLOADING,
    UNLOADED,
};

using ChunkID = int;

#endif //ASTGEN_CHUNKENUMS_H