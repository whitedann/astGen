//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_CHUNK_H
#define ASTGEN_CHUNK_H
#include <iostream>
#include <SFML/System/Vector2.hpp>
#include "ChunkEnums.h"
#include <SFML/Graphics.hpp>

#include "../TileTypes.h"

enum class TileType;

class Chunk {

public:

    Chunk(const ChunkID& l_cID, const sf::Vector2i& l_index):
        m_state(ChunkState::UNLOADED),
        m_id(l_cID),
        m_cIndex(l_index)
    {


    }

    ChunkID GetChunkID() const { return m_id; }
    ChunkState GetChunkState() const  { return m_state; }
    void SetChunkState(ChunkState l_chunkState)  { m_state = l_chunkState; }
    sf::Vector2i GetChunkIndex() const { return m_cIndex; }



private:

    std::atomic<ChunkState> m_state;
    ChunkID m_id;
    sf::Vector2i m_cIndex;

};

#endif //ASTGEN_CHUNK_H