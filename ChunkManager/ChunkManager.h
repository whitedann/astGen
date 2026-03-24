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
#include <SFML/Graphics/Text.hpp>

#include "Chunk.h"
#include "ChunkLoader.h"

static int MAX_CONCURRENT_LOADS = 5;
static int MAX_CONCURRENT_UNLOADS = 100;

static ChunkID MakeChunkID(const sf::Vector2i& l_cIndex) {
    return l_cIndex.x * 1000 + l_cIndex.y;
}

static sf::Vector2i MakeChunkIndex(const ChunkID l_cID) {
    return { l_cID / 1000, l_cID % 1000 };
}

static sf::Vector2f GetChunkWorldCenter(const ChunkID& id) {
    auto [x, y] = MakeChunkIndex(id);

    return {
        (x + 0.5f) * CHUNK_SIZE_PX,
        (y + 0.5f) * CHUNK_SIZE_PX
    };
}

static float DistanceSq(const sf::Vector2f& a, const sf::Vector2f& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

class ChunkManager {

public:

    ChunkManager(sf::RenderWindow* l_wind): m_window(l_wind) {
        bool loaded = m_font.openFromFile("Thintel.ttf");
        if(!loaded) {
            std::cerr<<"Error loading font"<<std::endl;
        }
        m_unloadQueuedSetText = new sf::Text(m_font, "");
        m_unloadQueuedSetText->setCharacterSize(32);
        m_unloadQueuedSetText->setFillColor(sf::Color::White);
        m_unloadQueuedSetText->setPosition({5.f, 5.f});

        m_unloadQueueText = new sf::Text(m_font, "");
        m_unloadQueueText->setCharacterSize(32);
        m_unloadQueueText->setFillColor(sf::Color::White);
        m_unloadQueueText->setPosition({5.f, 37.f});

        m_loadQueueText = new sf::Text(m_font, "");
        m_loadQueueText->setCharacterSize(32);
        m_loadQueueText->setFillColor(sf::Color::White);
        m_loadQueueText->setPosition({5.f, 69.f});

        m_loadQueuedSetText = new sf::Text(m_font, "");
        m_loadQueuedSetText->setCharacterSize(32);
        m_loadQueuedSetText->setFillColor(sf::Color::White);
        m_loadQueuedSetText->setPosition({5.f, 101.f});
    }

    void Update(float l_dT);
    void Draw(sf::RenderWindow* l_wind);

    void AddChunkLoader(ChunkLoader* l_chunkLoader) { m_chunkLoaders.push_back(l_chunkLoader); }

private:

    void FinishLoadTasks();
    void FinishUnloadTasks();
    void UpdateChunksToLoad();
    void UpdateChunksToUnload();

    void QueueLoadChunk(ChunkID l_cID);
    void QueueUnloadChunk(ChunkID l_cID);

    void CancelLoad(const ChunkID &l_cID);

    void CancelUnload(const ChunkID &l_cID);

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

    /** DEBUG **/
    sf::Font m_font;
    sf::Text* m_loadQueueText;
    sf::Text* m_loadQueuedSetText;
    sf::Text* m_unloadQueueText;
    sf::Text* m_unloadQueuedSetText;


};

#endif //ASTGEN_CHUNKMANAGER_H