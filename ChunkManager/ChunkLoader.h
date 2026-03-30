//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_CHUNKLOADER_H
#define ASTGEN_CHUNKLOADER_H
#include "Chunk.h"

class ChunkLoader {
public:

    virtual void LoadChunkAsync(ChunkID l_cID) = 0;
    virtual void UnloadChunkAsync(ChunkID l_cID) = 0;

    virtual void EndLoadChunk(ChunkID l_cID) = 0;
    virtual void EndUnloadChunk(ChunkID l_cID) = 0;

    //virtual void Redraw(Chunk& l_chunk);
    //virtual void Draw(Chunk& l_chunk, sf::RenderWindow* l_wind) = 0;


private:

};

#endif //ASTGEN_CHUNKLOADER_H