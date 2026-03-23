//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_CHUNKMANAGER_H
#define ASTGEN_CHUNKMANAGER_H
#include <deque>
#include <future>
#include <set>
#include <shared_mutex>
#include <unordered_set>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Chunk.h"
#include "ChunkLoader.h"

static int MAX_CONCURRENT_LOADS = 20;
static int MAX_CONCURRENT_UNLOADS = 20;

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

    void AddChunkLoader(ChunkLoader* l_chunkLoader) { m_chunkLoaders.push_back(l_chunkLoader); }

private:

    void UpdateLoadTasks();
    void UpdateUnloadTasks();
    void UpdateChunksToLoad();
    void UpdateChunksToUnload();

    void LoadChunk(ChunkID l_cID);
    void UnloadChunk(ChunkID l_cID);

    void CancelLoad(const ChunkID &l_cID);

    void ProcessLoadQueue();

    void ProcessUnloadQueue();

    void LoadChunkAsync(Chunk& l_chunk) const;
    void UnloadChunkAsync(Chunk& l_chunk) const;

    void EndLoadChunk(const ChunkID& l_cID) const;
    void EndUnloadChunk(const ChunkID& l_cID) const;

    void LockChunk(const ChunkID &l_cID);
    void UnlockChunk(const ChunkID &l_cID);
    bool ChunkIsLocked(const ChunkID &l_cID);

    sf::RenderWindow* m_window;

    std::unordered_map<ChunkID, std::shared_ptr<Chunk>> m_chunks;
    std::vector<ChunkLoader*> m_chunkLoaders;

    std::deque<ChunkID> m_loadQueue;
    std::unordered_set<ChunkID> m_loadQueuedSet;
    std::deque<ChunkID> m_unloadQueue;
    std::unordered_set<ChunkID> m_unloadQueuedSet;

    std::set<ChunkID> m_chunksToUnload;
    std::set<ChunkID> m_chunksLoaded;
    std::set<ChunkID> m_chunksInRange;

    std::unordered_map<ChunkID, std::future<void>> m_loadChunkTasks;
    std::unordered_map<ChunkID, std::future<void>> m_unloadChunkTasks;

    std::unordered_map<ChunkID, bool> m_chunkLocks;
    std::shared_mutex m_lockMutex;

};

#endif //ASTGEN_CHUNKMANAGER_H