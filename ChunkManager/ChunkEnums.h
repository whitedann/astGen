//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_CHUNKENUMS_H
#define ASTGEN_CHUNKENUMS_H

static constexpr int WORLD_SIZE_CHUNKS = 300;
static constexpr int CHUNK_SIZE_PX = 32;
static constexpr int TILE_SIZE_PX = 8;

enum class ChunkState {
    QUEUED_TO_LOAD,
    LOADING,
    LOADED,
    QUEUED_TO_UNLOAD,
    UNLOADING,
    UNLOADED,
};

using ChunkID = int64_t; // bigger than int to avoid collisions for large worlds

constexpr int CHUNK_HASH_BASE = 100000; // large enough to prevent collisions

inline ChunkID MakeChunkID(const sf::Vector2i& chunkIndex) {
    return static_cast<ChunkID>(chunkIndex.x) * CHUNK_HASH_BASE + chunkIndex.y;
}

inline sf::Vector2i MakeChunkIndex(ChunkID id) {
    return {
        static_cast<int>(id / CHUNK_HASH_BASE),  // x
        static_cast<int>(id % CHUNK_HASH_BASE)   // y
    };
}

#endif //ASTGEN_CHUNKENUMS_H