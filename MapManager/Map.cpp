//
// Created by Daniel White on 3/17/26.
//

#include "Map.h"

#include <iostream>
#include <ostream>
#include <SFML/Graphics/RectangleShape.hpp>
#include "../World.h"

void Map::LoadChunkAsync(Chunk& l_chunk) {
    sf::Vector2i chunkIndex = l_chunk.GetChunkIndex();

    for (int y = 0; y < CHUNK_SIZE_PX; y++) {
        for (int x = 0; x < CHUNK_SIZE_PX; x++) {
            // Convert local chunk coords to world coords
            int worldX = chunkIndex.x * CHUNK_SIZE_PX + x;
            int worldY = chunkIndex.y * CHUNK_SIZE_PX + y;
            auto tile = m_world->GetTile(worldX, worldY);
            sf::Vector2i tileIndex = {x, y};
            l_chunk.SetTile(tileIndex, tile.tileType);
        }
    }
}

void Map::UnloadChunkAsync(Chunk& l_chunk) {
    //std::cout << "UnloadChunkAsync" << std::endl;
}

void Map::Draw(Chunk& l_chunk, sf::RenderWindow* l_wind) {
    sf::Vector2i index = l_chunk.GetChunkIndex();
    sf::RectangleShape rect;
    rect.setFillColor({255, 0 , 0, 100});
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(1);
    rect.setSize({CHUNK_SIZE_PX, CHUNK_SIZE_PX});
    //auto currentView = l_wind->getView();
    //l_wind->setView(l_wind->getDefaultView());
    rect.setPosition({(float)(CHUNK_SIZE_PX * index.x), (float)(CHUNK_SIZE_PX * index.y) });
    l_wind->draw(rect);
    l_wind->draw(*l_chunk.GetSprite());
    //l_wind->setView(currentView);
}