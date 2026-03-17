//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_CHUNKLOADER_H
#define ASTGEN_CHUNKLOADER_H
#include "Chunk.h"

class ChunkLoader {
public:

    virtual void LoadChunkAsync(Chunk& l_chunk) = 0;
    virtual void UnloadChunkAsync(Chunk& l_chunk) = 0;

private:
};

#endif //ASTGEN_CHUNKLOADER_H