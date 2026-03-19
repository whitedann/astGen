//
// Created by Daniel White on 3/19/26.
//

#ifndef ASTGEN_MAPCHUNK_H
#define ASTGEN_MAPCHUNK_H
#include <memory>
#include <vector>
#include "../TileTypes.h"
#include "../ChunkManager/ChunkEnums.h"

class MapChunk {

public:

    MapChunk() {
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

    void SetTile(sf::Vector2i& l_tileIndex, TileType l_type) {
        int index = l_tileIndex.x * CHUNK_SIZE_PX + l_tileIndex.y;
        m_tiles[index] = l_type;
        m_tilesToRedraw.emplace_back(index, l_type);
    }

    bool NeedsRedraw() {
        return m_tilesToRedraw.size() > 0;
    }

private:
    TileType m_tiles[CHUNK_SIZE_PX * CHUNK_SIZE_PX];
    std::vector<std::pair<int,TileType>> m_tilesToRedraw;
    std::unique_ptr<sf::RenderTexture> m_texture;
    std::unique_ptr<sf::Sprite> m_sprite;
};

#endif //ASTGEN_MAPCHUNK_H