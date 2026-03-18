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

        m_texture = std::make_unique<sf::RenderTexture>();
        m_texture->resize({CHUNK_SIZE_PX, CHUNK_SIZE_PX});
        m_texture->clear({ 0, 0, 0, 0 });
        m_sprite = std::make_unique<sf::Sprite>(m_texture->getTexture());
        m_sprite->setTextureRect(sf::IntRect({ 0,0 }, { CHUNK_SIZE_PX, CHUNK_SIZE_PX }));
        m_sprite->setPosition(
        {static_cast<float>(m_cIndex.x * CHUNK_SIZE_PX),
            static_cast<float>(m_cIndex.y * CHUNK_SIZE_PX)}
        );
        m_sprite->setTexture(m_texture->getTexture());
    }

    ChunkID GetChunkID() const { return m_id; }
    ChunkState GetChunkState() const  { return m_state; }
    void SetChunkState(ChunkState l_chunkState)  { m_state = l_chunkState; }
    sf::Vector2i GetChunkIndex() const { return m_cIndex; }

    sf::Sprite* GetSprite() { return m_sprite.get(); }
    sf::RenderTexture* GetTexture() { return m_texture.get(); }

    void RedrawAll() {
        size_t totalVerts = m_tilesToRedraw.size() * 4;
        sf::VertexArray verts;
        verts.resize(totalVerts);
        int vertIndex = 0;
        for (auto [index, type] : m_tilesToRedraw) {
            //if (type == TileType::Empty) { continue; }
            int indexX = index % CHUNK_SIZE_PX;
            int indexY = index / CHUNK_SIZE_PX;
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
            vertIndex += 4;
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
        Draw(verts);
    }

    void Draw(sf::VertexArray& verts) {
        sf::RenderStates states = sf::RenderStates::Default;
        states.blendMode = sf::BlendNone;
        m_texture->draw(verts, states);
        m_texture->display();
        m_sprite->setTexture(m_texture->getTexture());
    }

    void SetTile(sf::Vector2i& l_tileIndex, TileType l_type) {
        int index = l_tileIndex.x * CHUNK_SIZE_PX + l_tileIndex.y;
        m_tiles[index] = l_type;
        m_tilesToRedraw.emplace_back(index, l_type);
    }

    bool NeedsRedraw() {
        return m_tilesToRedraw.size() > 0;
    }

private:

    ChunkState m_state;
    ChunkID m_id;
    sf::Vector2i m_cIndex;
    TileType m_tiles[CHUNK_SIZE_PX * CHUNK_SIZE_PX];
    std::vector<std::pair<int,TileType>> m_tilesToRedraw;
    std::unique_ptr<sf::RenderTexture> m_texture;
    std::unique_ptr<sf::Sprite> m_sprite;

};

#endif //ASTGEN_CHUNK_H