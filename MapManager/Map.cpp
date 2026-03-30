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
    for (int y = 0; y < CHUNK_SIZE_PX; y++) {
        for (int x = 0; x < CHUNK_SIZE_PX; x++) {
            // Convert local chunk coords to world coords
            int worldX = chunkIndex.x * CHUNK_SIZE_PX + x;
            int worldY = chunkIndex.y * CHUNK_SIZE_PX + y;
            auto tile = m_world->GetTile(worldX, worldY);
            sf::Vector2i tileIndex = {x, y};
            tempChunk->SetTile(tileIndex, tile.tileType);
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

void Map::Draw(sf::RenderWindow* l_wind) {
    for (auto& chunk : m_mapChunks) {
        if (chunk.second->NeedsRedraw()) {
            chunk.second->Redraw();
        }
        l_wind->draw(*chunk.second->GetSprite());

        sf::Vector2i index = chunk.second.get()->GetIndex();
        sf::RectangleShape rect;
        rect.setFillColor({0, 255 , 0, 40});
        rect.setOutlineColor({0, 0, 0, 100});
        rect.setOutlineThickness(1);
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
