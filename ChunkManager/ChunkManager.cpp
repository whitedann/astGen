#include "ChunkManager.h"

#include <iostream>

void ChunkManager::Update(float l_dT) {
    FinishLoadTasks();
    FinishUnloadTasks();

    ProcessLoadQueue();
    ProcessUnloadQueue();

    UpdateChunksToLoad();

    //UpdateTexts();
}

void ChunkManager::Draw(sf::RenderWindow *l_wind) {
    sf::View currentView = m_window->getView();
    l_wind->setView(m_window->getDefaultView());
    l_wind->draw(*m_text1);
    l_wind->draw(*m_text2);
    l_wind->draw(*m_text3);
    l_wind->draw(*m_text4);
    l_wind->setView(currentView);
}

void ChunkManager::UpdateTexts() {
    int countLoaded = 0;
    int countQueuedToLoad = 0;
    int countQueuedToUnload = 0;
    int countUnloading = 0;
    int countLoading = 0;

    for (auto& chunk : m_chunks) {
        auto state = chunk.second->GetChunkState();
        switch (state) {
            case ChunkState::LOADED: {
                countLoaded++;
                break;
            }
            case ChunkState::QUEUED_TO_LOAD: {
                countQueuedToLoad++;
                break;
            }
            case ChunkState::LOADING: {
                countLoading++;
                break;
            }
            case ChunkState::QUEUED_TO_UNLOAD: {
                countQueuedToUnload++;
                break;
            }
            case ChunkState::UNLOADING: {
                countUnloading++;
                break;
            }
        }
    }

    m_text1->setString("Loaded: " + std::to_string(countLoaded));
    m_text2->setString("Queued to load: " + std::to_string(countQueuedToLoad));
    m_text3->setString("Unload Queue Size: " + std::to_string(m_unloadQueue.size()));
    m_text4->setString("Queued to unload: " + std::to_string(countQueuedToUnload));
    m_text5->setString("Unloading: " + std::to_string(countUnloading));
}

void ChunkManager::FinishLoadTasks() {
    for (auto it = m_loadChunkTasks.begin(); it != m_loadChunkTasks.end();) {
        auto& task = it->second;
        if (task.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            Chunk* chunk = m_chunks[it->first].get();
            chunk->SetChunkState(ChunkState::LOADED);
            EndLoadChunk(it->first);
            it = m_loadChunkTasks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void ChunkManager::FinishUnloadTasks() {
    for (auto it = m_unloadChunkTasks.begin(); it != m_unloadChunkTasks.end();) {
        auto& [chunkID, task] = *it;
        if (task.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            Chunk* chunk = m_chunks[chunkID].get();
            chunk->SetChunkState(ChunkState::UNLOADED);
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

    int centerChunkX = static_cast<int>(std::floor(center.x / CHUNK_SIZE_PX));
    int centerChunkY = static_cast<int>(std::floor(center.y / CHUNK_SIZE_PX));

    int maxRadius = std::max({
        std::abs(centerChunkX - minChunkX),
        std::abs(centerChunkX - maxChunkX),
        std::abs(centerChunkY - minChunkY),
        std::abs(centerChunkY - maxChunkY)
    });

    // LOAD pass: expand outward in rings
    for (int r = 0; r <= maxRadius; r++) {
        int startX = centerChunkX - r;
        int endX   = centerChunkX + r;
        int startY = centerChunkY - r;
        int endY   = centerChunkY + r;

        auto tryLoad = [&](int x, int y) {
            if (x < minChunkX || x > maxChunkX || y < minChunkY || y > maxChunkY)
                return;

            ChunkID id = MakeChunkID({x, y});
            auto idx = MakeChunkIndex(id);

            auto [it, inserted] =
                m_chunks.try_emplace(id, std::make_shared<Chunk>(id, idx));

            Chunk& chunk = *it->second;

            if (chunk.GetChunkState() == ChunkState::UNLOADED) {
                QueueLoadChunk(id);
            }
            else if (chunk.GetChunkState() == ChunkState::QUEUED_TO_UNLOAD) {
                CancelUnload(id);
            }
        };

        // top
        for (int x = startX; x <= endX; x++)
            tryLoad(x, startY);

        // right
        for (int y = startY + 1; y <= endY; y++)
            tryLoad(endX, y);

        // bottom
        for (int x = endX - 1; x >= startX; x--)
            tryLoad(x, endY);

        // left
        for (int y = endY - 1; y > startY; y--)
            tryLoad(startX, y);
    }

    // UNLOAD pass: bounds check (no set lookup)
    for (auto& [id, chunk] : m_chunks) {
        auto [x, y] = MakeChunkIndex(id);

        if (x < minChunkX || x > maxChunkX || y < minChunkY || y > maxChunkY)
        {
            ChunkState state = chunk->GetChunkState();

            if (state == ChunkState::LOADED)
            {
                QueueUnloadChunk(id);
            }
            else if (state == ChunkState::QUEUED_TO_LOAD)
            {
                CancelLoad(id);
                chunk->SetChunkState(ChunkState::UNLOADED);
            }
        }
    }
}

void ChunkManager::QueueLoadChunk(ChunkID l_cID) {
    auto it = m_chunks.find(l_cID);
    if (it == m_chunks.end()) return;
    if (ChunkIsLocked(l_cID)) return;
    ChunkState state = it->second->GetChunkState();
    if (state == ChunkState::LOADING ||
        state == ChunkState::QUEUED_TO_LOAD ||
        state == ChunkState::LOADED) {
        return;
    }

    it->second->SetChunkState(ChunkState::QUEUED_TO_LOAD);
    m_loadQueue.push_back(l_cID);
}

void ChunkManager::QueueUnloadChunk(ChunkID l_cID) {
    auto it = m_chunks.find(l_cID);
    if (it == m_chunks.end()) return;

    ChunkState state = it->second->GetChunkState();

    if (state == ChunkState::UNLOADING || state == ChunkState::QUEUED_TO_UNLOAD || state == ChunkState::UNLOADED) {
        return;
    }

    if (state == ChunkState::QUEUED_TO_LOAD) {
        CancelLoad(l_cID);
        it->second->SetChunkState(ChunkState::UNLOADED);
        return;
    }
    if (ChunkIsLocked(l_cID)) return;

    it->second->SetChunkState(ChunkState::QUEUED_TO_UNLOAD);
    m_unloadQueue.push_back(l_cID);
}

void ChunkManager::CancelLoad(const ChunkID& l_cID) {
    m_loadQueue.erase(
        std::remove(m_loadQueue.begin(), m_loadQueue.end(), l_cID),
        m_loadQueue.end()
    );
    auto it = m_chunks.find(l_cID);
    if (it != m_chunks.end() &&
        it->second->GetChunkState() == ChunkState::QUEUED_TO_LOAD)
    {
        it->second->SetChunkState(ChunkState::UNLOADED);
    }
}

void ChunkManager::CancelUnload(const ChunkID& l_cID) {
    m_unloadQueue.erase(
        std::remove(m_unloadQueue.begin(), m_unloadQueue.end(), l_cID),
        m_unloadQueue.end()
    );
    auto it = m_chunks.find(l_cID);
    if (it != m_chunks.end() &&
        it->second->GetChunkState() == ChunkState::QUEUED_TO_UNLOAD)
    {
        it->second->SetChunkState(ChunkState::LOADED);
    }
}

void ChunkManager::ProcessLoadQueue() {
    while (!m_loadQueue.empty() &&
           m_loadChunkTasks.size() < MAX_CONCURRENT_LOADS)
    {
        ChunkID id = m_loadQueue.front();
        m_loadQueue.pop_front();

        auto it = m_chunks.find(id);
        if (it == m_chunks.end()) continue;

        auto& chunk = it->second;
        auto cID = chunk->GetChunkID();
        chunk->SetChunkState(ChunkState::LOADING);

        auto workTask = [cID, this]() {
            LoadChunkAsync(cID);
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

        auto it = m_chunks.find(id);
        if (it == m_chunks.end()) continue;
        auto& chunk = it->second;
        chunk->SetChunkState(ChunkState::UNLOADING);
        auto cID = chunk->GetChunkID();

        auto workTask = [cID, this]() {
            UnloadChunkAsync(cID);
        };

        m_unloadChunkTasks.insert({
            id,
            std::async(std::launch::async, workTask)
        });
    }
}

void ChunkManager::LoadChunkAsync(ChunkID l_cID) const {
    for (auto& loader : m_chunkLoaders) {
        loader->LoadChunkAsync(l_cID);
    }
}

void ChunkManager::UnloadChunkAsync(ChunkID l_cID) const {
    for (auto& loader : m_chunkLoaders) {
        loader->UnloadChunkAsync(l_cID);
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

bool ChunkManager::ChunkIsLocked(const ChunkID& l_cID) {
    auto it = m_chunks.find(l_cID);
    if (it == m_chunks.end()) return false;
    auto& chunk = it->second;
    auto state = chunk->GetChunkState();
    if (state != ChunkState::UNLOADED && state != ChunkState::LOADED) {
        return true;
    }
    return false;
}