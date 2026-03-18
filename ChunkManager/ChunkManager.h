//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_CHUNKMANAGER_H
#define ASTGEN_CHUNKMANAGER_H
#include <future>
#include <set>
#include <shared_mutex>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Chunk.h"
#include "ChunkLoader.h"

static ChunkID MakeChunkID(const sf::Vector2i& l_cIndex) {
    return l_cIndex.x * 1000 + l_cIndex.y;
}

static sf::Vector2i MakeChunkIndex(const ChunkID l_cID) {
    return { l_cID / 1000, l_cID % 1000 };
}

class ChunkManager {

public:

    ChunkManager(sf::RenderWindow* l_wind): m_window(l_wind) {}

    void Update(float l_dT);
    void Draw(sf::RenderWindow& l_wind);

    void AddChunkLoader(ChunkLoader* l_chunkLoader) { m_chunkLoaders.push_back(l_chunkLoader); }

private:

    void UpdateLoadTasks();
    void UpdateUnloadTasks();
    void UpdateChunksToLoad();
    void UpdateChunksToUnload();

    void LoadChunk(const ChunkID &l_cID);
    void UnloadChunk(const ChunkID &l_cID);
    void LoadChunkAsync(Chunk& l_chunk) const;
    void UnloadChunkAsync(Chunk& l_chunk) const;

    void LockChunk(const ChunkID &l_cID);
    void UnlockChunk(const ChunkID &l_cID);
    bool ChunkIsLocked(const ChunkID &l_cID);

    sf::RenderWindow* m_window;

    std::unordered_map<ChunkID, std::unique_ptr<Chunk>> m_chunks;
    std::vector<ChunkLoader*> m_chunkLoaders;
    std::set<ChunkID> m_chunksToUnload;
    std::set<ChunkID> m_chunksLoaded;
    std::set<ChunkID> m_chunksInRange;

    std::unordered_map<ChunkID, std::future<void>> m_loadChunkTasks;
    std::unordered_map<ChunkID, std::future<void>> m_unloadChunkTasks;

    std::unordered_map<ChunkID, bool> m_chunkLocks;
    std::shared_mutex m_lockMutex;

};

#endif //ASTGEN_CHUNKMANAGER_H