//
// Created by Daniel White on 3/19/26.
//

#ifndef ASTGEN_MAPCHUNK_H
#define ASTGEN_MAPCHUNK_H
#include <memory>
#include <vector>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "../TileTypes.h"
#include "../ChunkManager/ChunkEnums.h"

inline bool IsEdgeTileCardinal(TileType selfType, uint8_t neighborMask)
{
    if (selfType == TileType::Empty) return false;

    // lower 4 bits represent N,E,S,W
    return (neighborMask & 0x0F) != 0x0F;
}

inline bool IsEdgeTile(TileType selfType, uint8_t neighborMask)
{
    if (selfType == TileType::Empty) return false;

    // if any neighbor is NOT solid, then it's an edge tile
    return neighborMask != 0xFF;
}

inline sf::Color GetColorFromMaskCardinal(uint8_t mask) {
    // Treat the bottom 4 bits as N, E, S, W
    bool north = (mask & 0x01); // Bit 0
    bool east  = (mask & 0x02); // Bit 1
    bool south = (mask & 0x04); // Bit 2
    bool west  = (mask & 0x08); // Bit 3

    // Base color is dark gray so we can see when no bits are set
    uint8_t r = 50;
    uint8_t g = 50;
    uint8_t b = 50;

    // Each direction adds a distinct amount of a color channel
    if (north) r += 150;
    if (east)  g += 150;
    if (south) b += 150;

    // West adds a mix to make Yellow/Orange
    if (west) {
        r += 50;
        g += 50;
    }

    return sf::Color(r, g, b);
}

class MapChunk {

public:

    MapChunk(const ChunkID& l_cID, const sf::Vector2i& l_cIndex) {
        m_chunkID = l_cID;
        m_cIndex = l_cIndex;
    }

    void SetupTexture() {
        // Can only call this on main thread!
        m_texture = std::make_unique<sf::RenderTexture>();
        m_texture->resize({CHUNK_SIZE_PX, CHUNK_SIZE_PX});
        m_texture->clear({ 0, 0, 0, 0 });
        m_sprite = std::make_unique<sf::Sprite>(m_texture->getTexture());
        m_sprite->setTextureRect(sf::IntRect({ 0,0 }, { CHUNK_SIZE_PX, CHUNK_SIZE_PX}));
        m_sprite->setPosition(
        {static_cast<float>(m_cIndex.x * CHUNK_SIZE_PX),
            static_cast<float>(m_cIndex.y * CHUNK_SIZE_PX)}
        );
        m_sprite->setTexture(m_texture->getTexture());
    }

    sf::Sprite* GetSprite() { return m_sprite.get(); }
    sf::RenderTexture* GetTexture() { return m_texture.get(); }

    void SetTile(sf::Vector2i& l_tileIndex, Tile l_type, bool l_redraw = false) {
        int index = l_tileIndex.y * CHUNK_SIZE_PX + l_tileIndex.x;
        m_tiles[index] = l_type;
        if (l_redraw) {
            m_tilesIndexesToRedraw.emplace_back(index);
        }
    }

    Tile& GetTile(const sf::Vector2i& l_tileIndex) {
        int index = l_tileIndex.y * CHUNK_SIZE_PX + l_tileIndex.x;
        return m_tiles[index];
    }

    void Redraw() {
        m_texture->setSmooth(false);    
        size_t totalVerts = m_tilesIndexesToRedraw.size() * 4;
        sf::VertexArray verts;
        verts.resize(totalVerts);
        int vertIndex = 0;
        for (auto index : m_tilesIndexesToRedraw) {
            auto& tile =  m_tiles[index];
            //if (type == TileType::Empty) { continue; }
            int indexX = index % CHUNK_SIZE_PX;
            int indexY = index / CHUNK_SIZE_PX;
            sf::Vertex* quad = &verts[vertIndex];
            sf::Color tileColor;
            switch (tile.tileType) {
                case TileType::Empty:        tileColor = sf::Color::Transparent; break;
                case TileType::Dust:         tileColor = sf::Color(200,200,200); break;
                case TileType::Rock:         tileColor = sf::Color(100,100,100); break;
                case TileType::DenseRock:    tileColor = sf::Color(50,50,50); break;
                case TileType::SuperDenseRock: tileColor = sf::Color::Green; break;
                default:                     tileColor = sf::Color::Magenta; break;
            }
            if (IsEdgeTileCardinal(tile.tileType, tile.geometry)) {
                //tileColor = sf::Color::Green;
            }
            //tileColor = GetColorFromMaskCardinal(tile.geometry);
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
        m_tilesIndexesToRedraw.clear();
        sf::RenderStates states = sf::RenderStates::Default;
        states.blendMode = sf::BlendNone;
        m_texture->clear(sf::Color::Transparent);
        m_texture->draw(verts, states);
        m_texture->display();
        m_sprite->setTexture(m_texture->getTexture());
    }

    sf::Vector2i GetIndex() {
        return m_cIndex;
    }

    bool NeedsRedraw() {
        return m_tilesIndexesToRedraw.size() > 0;
    }

private:
    sf::Vector2i m_cIndex;
    ChunkID m_chunkID;
    Tile m_tiles[CHUNK_SIZE_PX * CHUNK_SIZE_PX];
    std::vector<int> m_tilesIndexesToRedraw;
    std::unique_ptr<sf::RenderTexture> m_texture;
    std::unique_ptr<sf::Sprite> m_sprite;
};



#endif //ASTGEN_MAPCHUNK_H