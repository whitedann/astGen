//
// Created by Daniel White on 3/17/26.
//

#include "ChunkManager.h"

#include <iostream>

void ChunkManager::Update(float l_dT) {
    UpdateLoadTasks();
    UpdateUnloadTasks();
    UpdateChunksToLoad();
    UpdateChunksToUnload();
}

void ChunkManager::UpdateLoadTasks() {
    for (auto it = m_loadChunkTasks.begin(); it != m_loadChunkTasks.end();) {
        auto& task = it->second;
        if (task.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            //Chunk* chunk = m_chunks[it->first].get();
            //EndLoadChunk(*chunk);
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
        auto& task = it->second;
        if (task.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            it = m_unloadChunkTasks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ChunkManager::UpdateChunksToLoad() {
    m_chunksInRange.clear();

    const sf::Vector2f center = m_currentView.getCenter();
    const sf::Vector2f size   = m_currentView.getSize();

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
                std::make_unique<Chunk>(cID, cIndex)
            );

        Chunk& chunk = *it->second;
        if (chunk.GetChunkState() == ChunkState::UNLOADED) {
            LoadChunk(chunk.GetChunkID());
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

void ChunkManager::LoadChunk(const ChunkID& l_cID) {
    auto& chunk = m_chunks[l_cID];
    LockChunk(l_cID);
    chunk->SetChunkState(ChunkState::LOADING);
    //PreloadChunk(*chunk.get());
    auto workTask = [&chunk, this]() {
        LoadChunkAsync(*chunk.get());
        chunk->SetChunkState(ChunkState::LOADED);
        UnlockChunk(chunk->GetChunkID());
    };
    m_loadChunkTasks.insert({ l_cID, std::async(std::launch::async, workTask) });
}

void ChunkManager::UnloadChunk(const ChunkID& l_cID) {
    auto& chunk = m_chunks[l_cID];
    LockChunk(l_cID);
    chunk->SetChunkState(ChunkState::UNLOADING);
    m_chunksLoaded.erase(l_cID);
    //PreUnloadChunk(*chunk.get());
    auto workTask = [&chunk, this]() {
        UnloadChunkAsync(*chunk.get());
        chunk->SetChunkState(ChunkState::UNLOADED);
        UnlockChunk(chunk->GetChunkID());
    };
    m_unloadChunkTasks.insert({ l_cID, std::async(std::launch::async, workTask) });
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

void ChunkManager::Draw(sf::RenderWindow& l_wind) {

}


