//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_MAP_H
#define ASTGEN_MAP_H
#include <mutex>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "MapChunk.h"
#include "../ChunkManager/ChunkLoader.h"
#include "../ChunkManager/Chunk.h"

class World;

class Map : public ChunkLoader {

public:

    Map(World* l_world): m_world(l_world) {}

    void LoadChunkAsync(ChunkID l_cID) override;
    void UnloadChunkAsync(ChunkID l_cID) override;
    void EndLoadChunk(ChunkID l_cID) override;
    void EndUnloadChunk(ChunkID l_cID) override;

    void Update(float l_dT);
    void Draw(sf::RenderWindow* l_wind);

private:

    MapChunk& AddTempChunk(const ChunkID& l_chunk, const sf::Vector2i& l_cIndex);

    void RemoveChunk(const ChunkID& l_chunk);

    World* m_world;

    std::mutex m_tempMutex;
    std::unordered_map<ChunkID, std::unique_ptr<MapChunk>> m_tempMapChunkData;
    std::unordered_map<ChunkID, std::unique_ptr<MapChunk>> m_mapChunks;

};

#endif //ASTGEN_MAP_H