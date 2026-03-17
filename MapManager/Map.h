//
// Created by Daniel White on 3/17/26.
//

#ifndef ASTGEN_MAP_H
#define ASTGEN_MAP_H
#include "../ChunkManager/ChunkLoader.h"
#include "../ChunkManager/Chunk.h"

class Map : public ChunkLoader {

public:

    void LoadChunkAsync(Chunk& l_chunk);
    void UnloadChunkAsync(Chunk& l_chunk);

private:

};

#endif //ASTGEN_MAP_H