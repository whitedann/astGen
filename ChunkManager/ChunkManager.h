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

static int MAX_CONCURRENT_LOADS = 250;
static int MAX_CONCURRENT_UNLOADS = 200;

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
        m_text1 = new sf::Text(m_font, "");
        m_text1->setCharacterSize(32);
        m_text1->setFillColor(sf::Color::White);
        m_text1->setPosition({10.f, 5.f});

        m_text2 = new sf::Text(m_font, "");
        m_text2->setCharacterSize(32);
        m_text2->setFillColor(sf::Color::Black);
        m_text2->setPosition({10.f, 37.f});

        m_text3 = new sf::Text(m_font, "");
        m_text3->setCharacterSize(32);
        m_text3->setFillColor(sf::Color::Black);
        m_text3->setPosition({10.f, 69.f});

        m_text4 = new sf::Text(m_font, "");
        m_text4->setCharacterSize(32);
        m_text4->setFillColor(sf::Color::Black);
        m_text4->setPosition({10.f, 101.f});

        m_text5 = new sf::Text(m_font, "");
        m_text5->setCharacterSize(32);
        m_text5->setFillColor(sf::Color::Black);
        m_text5->setPosition({10.f, 133.f});

        m_text6 = new sf::Text(m_font, "");
        m_text6->setCharacterSize(32);
        m_text6->setFillColor(sf::Color::Black);
        m_text6->setPosition({10.f, 165.f});

    }

    void Update(float l_dT);
    void Draw(sf::RenderWindow* l_wind);

    void AddChunkLoader(ChunkLoader* l_chunkLoader) { m_chunkLoaders.push_back(l_chunkLoader); }

    void SetText5(std::string text);
    void SetText6(std::string l_text);

private:

    void FinishLoadTasks();
    void FinishUnloadTasks();
    void UpdateChunksToLoad();

    void QueueLoadChunk(ChunkID l_cID);
    void QueueUnloadChunk(ChunkID l_cID);

    void CancelLoad(const ChunkID &l_cID);

    void CancelUnload(const ChunkID &l_cID);

    void ProcessLoadQueue();
    void ProcessUnloadQueue();

    void LoadChunkAsync(ChunkID l_cID) const;
    void UnloadChunkAsync(ChunkID l_cID) const;

    void EndLoadChunk(const ChunkID& l_cID) const;
    void EndUnloadChunk(const ChunkID& l_cID) const;

    bool ChunkIsLocked(const ChunkID &l_cID);

    // DEBUG
    void UpdateTexts();

    sf::RenderWindow* m_window;

    std::unordered_map<ChunkID, std::shared_ptr<Chunk>> m_chunks;
    std::vector<ChunkLoader*> m_chunkLoaders;

    std::deque<ChunkID> m_loadQueue;
    std::deque<ChunkID> m_unloadQueue;

    std::unordered_map<ChunkID, std::future<void>> m_loadChunkTasks;
    std::unordered_map<ChunkID, std::future<void>> m_unloadChunkTasks;

    /** DEBUG **/
    sf::Font m_font;
    sf::Text* m_text1;
    sf::Text* m_text2;
    sf::Text* m_text3;
    sf::Text* m_text4;
    sf::Text* m_text5;

    sf::Text* m_text6;
};

#endif //ASTGEN_CHUNKMANAGER_H