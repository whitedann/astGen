//
// Created by Daniel White on 3/17/26.
//

#include "Map.h"

#include <iostream>
#include <ostream>
#include <SFML/Graphics/RectangleShape.hpp>
#include "../World.h"
#include "../ChunkManager/ChunkManager.h"

void Map::Update(float l_dT) {

}

void Map::LoadChunkAsync(ChunkID l_cID) {
    sf::Vector2i chunkIndex = MakeChunkIndex(l_cID);
    std::unique_ptr<MapChunk> tempChunk =
        std::make_unique<MapChunk>(
        l_cID,
        chunkIndex);
    // PASS 1: generate base tiles
    for (int y = 0; y < CHUNK_SIZE_PX; y++) {
        for (int x = 0; x < CHUNK_SIZE_PX; x++) {
            // Convert local chunk coords to world coords
            int worldX = chunkIndex.x * CHUNK_SIZE_PX + x;
            int worldY = chunkIndex.y * CHUNK_SIZE_PX + y;
            auto tile = m_world->GetTile(worldX, worldY);
            sf::Vector2i tileIndex = {x, y};
            tempChunk->SetTile(tileIndex, tile);
        }
    }
    // PASS 2: compute geometry bitmask for each tile
    for (int y = 0; y < CHUNK_SIZE_PX; y++) {
        for (int x = 0; x < CHUNK_SIZE_PX; x++) {
            sf::Vector2i tileIndex = {x, y};
            Tile tile = tempChunk->GetTile(tileIndex);
            if (tile.tileType == TileType::Empty) {
                tile.geometry = 0;
            }
            else {
                uint8_t mask = ComputeSolidMask(*tempChunk, chunkIndex, x, y);
                tile.geometry = mask;
            }
            tempChunk->SetTile(tileIndex, tile, true);
        }
    }
    {
        std::lock_guard lock(m_tempMutex);
        m_tempMapChunkData.emplace(l_cID, std::move(tempChunk));
    }
}

void Map::EndLoadChunk(ChunkID l_cID) {
    std::unique_ptr<MapChunk> tempChunk;
    {
        std::lock_guard lock(m_tempMutex);

        auto it = m_tempMapChunkData.find(l_cID);
        if (it == m_tempMapChunkData.end())
            return; // nothing ready yet

        tempChunk = std::move(it->second);
        m_tempMapChunkData.erase(it);
    }

    auto it = m_mapChunks.find(l_cID);
    if (it != m_mapChunks.end())
    {
        it->second = std::move(tempChunk);
    }
    else
    {
        tempChunk->SetupTexture();
        m_mapChunks.emplace(l_cID, std::move(tempChunk));
    }}

void Map::UnloadChunkAsync(ChunkID l_cID) {

}

void Map::EndUnloadChunk(ChunkID l_cID) {
    RemoveChunk(l_cID);
}

uint8_t Map::GetSolidMask(const sf::Vector2f& globalPos)
{
    // Convert pixel position to integer tile coordinate
    int tx = static_cast<int>(std::floor(globalPos.x));
    int ty = static_cast<int>(std::floor(globalPos.y));

    auto isSolid = [](TileType t) {
        return t != TileType::Empty;
    };

    bool N  = isSolid(m_world->GetTile(tx,     ty - 1).tileType);
    bool E  = isSolid(m_world->GetTile(tx + 1, ty    ).tileType);
    bool S  = isSolid(m_world->GetTile(tx,     ty + 1).tileType);
    bool W  = isSolid(m_world->GetTile(tx - 1, ty    ).tileType);

    bool NE = N && E && isSolid(m_world->GetTile(tx + 1, ty - 1).tileType);
    bool SE = S && E && isSolid(m_world->GetTile(tx + 1, ty + 1).tileType);
    bool SW = S && W && isSolid(m_world->GetTile(tx - 1, ty + 1).tileType);
    bool NW = N && W && isSolid(m_world->GetTile(tx - 1, ty - 1).tileType);

    uint8_t mask = 0;
    if (N)  mask |= (1 << 0);
    if (E)  mask |= (1 << 1);
    if (S)  mask |= (1 << 2);
    if (W)  mask |= (1 << 3);
    if (NE) mask |= (1 << 4);
    if (SE) mask |= (1 << 5);
    if (SW) mask |= (1 << 6);
    if (NW) mask |= (1 << 7);

    return mask;
}

uint8_t Map::ComputeSolidMask(MapChunk& chunk, const sf::Vector2i& chunkIndex, int x, int y)
{
    auto isSolid = [](TileType t) {
        return t != TileType::Empty;
    };

    bool N  = isSolid(GetTileForMask(chunk, chunkIndex, x,     y - 1).tileType);
    bool E  = isSolid(GetTileForMask(chunk, chunkIndex, x + 1, y    ).tileType);
    bool S  = isSolid(GetTileForMask(chunk, chunkIndex, x,     y + 1).tileType);
    bool W  = isSolid(GetTileForMask(chunk, chunkIndex, x - 1, y    ).tileType);

    bool NE = N && E && isSolid(GetTileForMask(chunk, chunkIndex, x + 1, y - 1).tileType);
    bool SE = S && E && isSolid(GetTileForMask(chunk, chunkIndex, x + 1, y + 1).tileType);
    bool SW = S && W && isSolid(GetTileForMask(chunk, chunkIndex, x - 1, y + 1).tileType);
    bool NW = N && W && isSolid(GetTileForMask(chunk, chunkIndex, x - 1, y - 1).tileType);

    uint8_t mask = 0;
    if (N)  mask |= (1 << 0);
    if (E)  mask |= (1 << 1);
    if (S)  mask |= (1 << 2);
    if (W)  mask |= (1 << 3);
    if (NE) mask |= (1 << 4);
    if (SE) mask |= (1 << 5);
    if (SW) mask |= (1 << 6);
    if (NW) mask |= (1 << 7);

    return mask;
}

Tile Map::GetTileForMask(MapChunk& chunk, const sf::Vector2i& chunkIndex, int localX, int localY)
{
    if (localX >= 0 && localX < CHUNK_SIZE_PX && localY >= 0 && localY < CHUNK_SIZE_PX)
    {
        sf::Vector2i index(localX, localY);
        return chunk.GetTile(index);
    }

    // outside this chunk -> query world
    int worldX = chunkIndex.x * CHUNK_SIZE_PX + localX;
    int worldY = chunkIndex.y * CHUNK_SIZE_PX + localY;

    return m_world->GetTile(worldX, worldY);
}

void Map::Draw(sf::RenderWindow* l_wind) {
    for (auto& chunk : m_mapChunks) {
        if (chunk.second->NeedsRedraw()) {
            chunk.second->Redraw();
        }
        l_wind->draw(*chunk.second->GetSprite());

        sf::Vector2i index = chunk.second.get()->GetIndex();
        sf::RectangleShape rect;
        rect.setFillColor({0, 255 , 0, 0});
        rect.setOutlineColor({0, 0, 0, 100});
        rect.setOutlineThickness(1.f);
        rect.setSize({CHUNK_SIZE_PX - 2.f, CHUNK_SIZE_PX - 2.f});
        rect.setPosition({(float)(CHUNK_SIZE_PX * index.x) + 1.f, (float)(CHUNK_SIZE_PX * index.y) + 1.f });
        l_wind->draw(rect);
    }
}

MapChunk& Map::AddTempChunk(const ChunkID &l_chunkID, const sf::Vector2i& l_cIndex) {
    std::lock_guard<std::mutex> lock(m_tempMutex);

    auto it = m_tempMapChunkData.find(l_chunkID);
    if (it != m_tempMapChunkData.end())
        return *(it->second);

    auto [insertIt, success] = m_tempMapChunkData.emplace(
        l_chunkID,
        std::make_unique<MapChunk>(l_chunkID, l_cIndex)
    );
    return *(insertIt->second);
}

void Map::RemoveChunk(const ChunkID &l_chunk) {
    m_mapChunks.erase(l_chunk);
}
