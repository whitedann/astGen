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

void Map::Redraw(Chunk& l_chunk) {
    size_t totalVerts = m_tilesToRedraw.size() * 4;
    sf::VertexArray verts;
    verts.resize(totalVerts);
    int vertIndex = 0;
    for (auto [index, type] : m_tilesToRedraw) {
        //if (type == TileType::Empty) { continue; }
        int indexX = index / CHUNK_SIZE_PX;
        int indexY = index % CHUNK_SIZE_PX;
        sf::Vertex* quad = &verts[vertIndex];
        sf::Color tileColor;
        switch (type) {
            case TileType::Empty:        tileColor = sf::Color::Transparent; break;
            case TileType::Dust:         tileColor = sf::Color(200,200,200); break;
            case TileType::Rock:         tileColor = sf::Color(100,100,100); break;
            case TileType::DenseRock:    tileColor = sf::Color(50,50,50); break;
            case TileType::SuperDenseRock: tileColor = sf::Color::Green; break;
            default:                     tileColor = sf::Color::Magenta; break;
        }
        sf::Vector2i tilePos = { indexX, indexY };
        quad[0].position = { (float)(tilePos.x), (float)(tilePos.y)};
        quad[1].position = { (float)((tilePos.x + 1)), (float)(tilePos.y)};
        quad[2].position = { (float)((tilePos.x + 1)), (float)((tilePos.y + 1))};
        quad[3].position = { (float)(tilePos.x), (float)((tilePos.y + 1))};
        quad[0].color = tileColor;
        quad[1].color = tileColor;
        quad[2].color = tileColor;
        quad[3].color = tileColor;
        vertIndex += 4;
    }
    m_tilesToRedraw.clear();
    sf::RenderStates states = sf::RenderStates::Default;
    states.blendMode = sf::BlendNone;
    m_texture->draw(verts, states);
    m_texture->display();
    m_sprite->setTexture(m_texture->getTexture());
}

void Map::Draw(Chunk& l_chunk, sf::RenderWindow* l_wind) {
    l_wind->draw(*l_chunk.GetSprite());

    sf::Vector2i index = l_chunk.GetChunkIndex();
    sf::RectangleShape rect;
    rect.setFillColor({0, 255 , 0, 20});
    rect.setOutlineColor({0, 0, 0, 100});
    rect.setOutlineThickness(2);
    rect.setSize({CHUNK_SIZE_PX - 4.f, CHUNK_SIZE_PX - 4.f});
    rect.setPosition({(float)(CHUNK_SIZE_PX * index.x) + 2.f, (float)(CHUNK_SIZE_PX * index.y) + 2.f });
    l_wind->draw(rect);
}