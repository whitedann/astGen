//
// Created by Daniel White on 3/5/26.
//

#ifndef ASTGEN_WORLD_H
#define ASTGEN_WORLD_H
#include <cstdint>
#include <array>
#include <random>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

class World {
public:
    World(sf::Vector2u size)
        : m_size(size),
          m_center(size.x / 2, size.y / 2),
          m_image(size, sf::Color::Black),
         m_sprite(nullptr)
    {
        Generate();
        InitPerlin(42);
    }

    void Draw(sf::RenderWindow& window)
    {
        window.draw(*m_sprite);
    }

    sf::Vector2u GetSize() const { return m_size; }

private:
    void Generate()
    {
        for (unsigned int y = 0; y < m_size.y; y++)
        {
            for (unsigned int x = 0; x < m_size.x; x++)
            {
                sf::Vector2f dist = { (float)m_center.x - x, (float)m_center.y - y };
                float magnitude = dist.length();
                if (magnitude < 5000)
                {
                    m_image.setPixel({x,y}, {200,200,200,200});
                }
            }
        }
        auto completed = m_texture.resize(m_size);
        m_texture.update(m_image);
        m_sprite = new sf::Sprite(m_texture);
    }

    void InitPerlin(uint32_t seed) {
        std::array<int,256> p;

        for (int i = 0; i < 256; i++)
            p[i] = i;

        std::mt19937 rng(seed);
        std::shuffle(p.begin(), p.end(), rng);

        for (int i = 0; i < 256; i++)
        {
            m_perm[i] = p[i];
            m_perm[i + 256] = p[i];
        }
    }

    sf::Vector2u m_size;
    sf::Vector2u m_center;
    sf::Image m_image;
    sf::Texture m_texture;
    sf::Sprite* m_sprite;
    std::array<int, 512> m_perm;
};


#endif //ASTGEN_WORLD_H