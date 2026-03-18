//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_MAP_H
#define ASTGEN_MAP_H
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "../ChunkManager/ChunkLoader.h"
#include "../ChunkManager/Chunk.h"

class World;

class Map : public ChunkLoader {

public:

    Map(World* l_world): m_world(l_world) {}

    void LoadChunkAsync(Chunk& l_chunk) override;
    void UnloadChunkAsync(Chunk& l_chunk) override;
    void Draw(Chunk& l_chunk, sf::RenderWindow* l_wind) override;

private:

    World* m_world;
    std::unordered_map<ChunkID, sf::Sprite*> m_mapChunks;

};

#endif //ASTGEN_MAP_H