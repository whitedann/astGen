//
// Created by Daniel White on 3/17/26.
//

#include "ChunkManager.h"

#include <iostream>

void ChunkManager::Update(float l_dT) {
    UpdateLoadTasks();
    UpdateUnloadTasks();

    ProcessLoadQueue();
    ProcessUnloadQueue();

    UpdateChunksToLoad();
    UpdateChunksToUnload();
}

void ChunkManager::UpdateLoadTasks() {
    for (auto it = m_loadChunkTasks.begin(); it != m_loadChunkTasks.end();) {
        auto& task = it->second;
        if (task.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            //Chunk* chunk = m_chunks[it->first].get();
            EndLoadChunk(it->first);
            m_chunksLoaded.insert(it->first);
            it = m_loadChunkTasks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ChunkManager::UpdateUnloadTasks() {
    for (auto it = m_unloadChunkTasks.begin(); it != m_unloadChunkTasks.end();) {
        auto& [chunkID, task] = *it;
        if (task.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            EndUnloadChunk(chunkID);
            m_chunks.erase(chunkID);
            it = m_unloadChunkTasks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ChunkManager::UpdateChunksToLoad() {
    m_chunksInRange.clear();

    const sf::Vector2f center = m_window->getView().getCenter();
    const sf::Vector2f size   = m_window->getView().getSize();

    const float left   = center.x - size.x * 0.5f;
    const float right  = center.x + size.x * 0.5f;
    const float top    = center.y - size.y * 0.5f;
    const float bottom = center.y + size.y * 0.5f;

    int minChunkX = static_cast<int>(std::floor(left / CHUNK_SIZE_PX));
    int maxChunkX = static_cast<int>(std::floor(right / CHUNK_SIZE_PX));
    int minChunkY = static_cast<int>(std::floor(top / CHUNK_SIZE_PX));
    int maxChunkY = static_cast<int>(std::floor(bottom / CHUNK_SIZE_PX));

    constexpr int margin = 1;

    minChunkX -= margin;
    maxChunkX += margin;
    minChunkY -= margin;
    maxChunkY += margin;

    for (int x = minChunkX; x <= maxChunkX; x++)
    {
        for (int y = minChunkY; y <= maxChunkY; y++)
        {
            ChunkID id = MakeChunkID({x, y});
            m_chunksInRange.insert(id);
        }
    }

    for (auto& cID: m_chunksInRange) {
        auto cIndex = MakeChunkIndex(cID);
        auto [it, inserted] =
            m_chunks.try_emplace(
                cID,
                std::make_shared<Chunk>(cID, cIndex)
            );

        Chunk& chunk = *it->second;
        if (chunk.GetChunkState() == ChunkState::UNLOADED) {
            LoadChunk(chunk.GetChunkID());
        }
    }
    for (auto& cID : m_chunksToUnload) {
        if (!ChunkIsLocked(cID)) {
            UnloadChunk(cID);
        }
    }
}

void ChunkManager::UpdateChunksToUnload() {
    m_chunksToUnload.clear();
    for (auto& loadedChunk : m_chunksLoaded) {
        auto result = m_chunksInRange.find(loadedChunk);
        if (result == m_chunksInRange.end()) {
            m_chunksToUnload.insert(loadedChunk);
        }
    }
}

void ChunkManager::LoadChunk(ChunkID l_cID) {

    if (ChunkIsLocked(l_cID)) return;
    if (m_loadQueuedSet.contains(l_cID)) return;

    //std::cout << "Loading " << l_cID << std::endl;

    LockChunk(l_cID);
    m_loadQueue.push_back(l_cID);
    m_loadQueuedSet.insert(l_cID);
}

void ChunkManager::UnloadChunk(ChunkID l_cID) {
    if (m_loadQueuedSet.contains(l_cID)) {
        CancelLoad(l_cID);
        return;
    }
    if (m_unloadQueuedSet.contains(l_cID)) return;

    if (ChunkIsLocked(l_cID)) return;

    //std::cout << "Unloading " << l_cID << std::endl;

    LockChunk(l_cID);
    m_unloadQueue.push_back(l_cID);
    m_unloadQueuedSet.insert(l_cID);
}

void ChunkManager::CancelLoad(const ChunkID& l_cID) {
    if (!m_loadQueuedSet.contains(l_cID)) return;

    m_loadQueuedSet.erase(l_cID);

    // remove from deque (linear, but fine unless huge)
    m_loadQueue.erase(
        std::remove(m_loadQueue.begin(), m_loadQueue.end(), l_cID),
        m_loadQueue.end()
    );

    UnlockChunk(l_cID);
}

void ChunkManager::ProcessLoadQueue() {
    while (!m_loadQueue.empty() &&
           m_loadChunkTasks.size() < MAX_CONCURRENT_LOADS)
    {
        ChunkID id = m_loadQueue.front();
        m_loadQueue.pop_front();
        m_loadQueuedSet.erase(id);

        auto chunk = m_chunks[id];
        chunk->SetChunkState(ChunkState::LOADING);

        auto workTask = [chunk, this]() {
            LoadChunkAsync(*chunk);
            chunk->SetChunkState(ChunkState::LOADED);
            UnlockChunk(chunk->GetChunkID());
        };

        m_loadChunkTasks.insert({
            id,
            std::async(std::launch::async, workTask)
        });
    }
}

void ChunkManager::ProcessUnloadQueue() {
    while (!m_unloadQueue.empty() &&
           m_unloadChunkTasks.size() < MAX_CONCURRENT_UNLOADS)
    {
        ChunkID id = m_unloadQueue.front();
        m_unloadQueue.pop_front();

        auto chunk = m_chunks[id];
        chunk->SetChunkState(ChunkState::UNLOADING);
        m_chunksLoaded.erase(id);

        auto workTask = [chunk, this]() {
            UnloadChunkAsync(*chunk);
            chunk->SetChunkState(ChunkState::UNLOADED);
            UnlockChunk(chunk->GetChunkID());
        };

        m_unloadChunkTasks.insert({
            id,
            std::async(std::launch::async, workTask)
        });
    }
}

void ChunkManager::LoadChunkAsync(Chunk& l_chunk) const {
    for (auto& loader : m_chunkLoaders) {
        loader->LoadChunkAsync(l_chunk);
    }
}

void ChunkManager::UnloadChunkAsync(Chunk& l_chunk) const {
    for (auto& loader : m_chunkLoaders) {
        loader->UnloadChunkAsync(l_chunk);
    }
}

void ChunkManager::EndLoadChunk(const ChunkID& l_cID) const {
    for (auto& loader : m_chunkLoaders) {
        loader->EndLoadChunk(l_cID);
    }
}

void ChunkManager::EndUnloadChunk(const ChunkID& l_cID) const {
    for (auto &loader : m_chunkLoaders) {
        loader->EndUnloadChunk(l_cID);
    }
}

void ChunkManager::LockChunk(const ChunkID& l_cID) {
    std::unique_lock lock(m_lockMutex);
    m_chunkLocks[l_cID] = true;
}

void ChunkManager::UnlockChunk(const ChunkID& l_cID) {
    std::unique_lock lock(m_lockMutex);
    m_chunkLocks[l_cID] = false;
}

bool ChunkManager::ChunkIsLocked(const ChunkID& l_cID) {
    std::shared_lock lock(m_lockMutex);
    auto entry = m_chunkLocks.find(l_cID);
    if (entry == m_chunkLocks.end()) { return false; }
    return entry->second;
}



