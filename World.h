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

#include "TileTypes.h"

class World {
public:
    World(sf::Vector2u size)
        : m_size(size),
          m_center(size.x / 2, size.y / 2),
          m_image(size, sf::Color::Black),
         m_sprite(nullptr)
    {
        InitPerlin(42);
        Generate();
    }

    TileType GetTile(int x, int y) {
        float dx = static_cast<float>(x - m_center.x);
        float dy = static_cast<float>(y - m_center.y);

        float distance = std::sqrt(dx * dx + dy * dy);

        float surfaceNoiseScale = 0.02f;
        float surfaceNoise = SamplePerlin(x * surfaceNoiseScale, y * surfaceNoiseScale);
        float ringNoiseScale = 0.05f;
        float ringNoise    = SamplePerlin(x * ringNoiseScale, y * ringNoiseScale);

        float baseRadius = 3900.f;
        float amplitude = 20.f;

        float surfaceRadius = baseRadius + surfaceNoise * amplitude;
        if (distance > surfaceRadius) {
            return TileType::Empty;
        }

        float ringDistance = distance + ringNoise * 300.f;
        float t = distance / ringDistance;
        float normT = t / baseRadius;
        normT += ringNoise * .1;

        if (normT > 0.8f) {
            return TileType::Dust;
        }
        if (normT > 0.6f) {
            return TileType::Rock;
        }
        if (normT > 0.0f){
            return TileType::DenseRock;
        }
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
                TileType tileType = GetTile(x, y);
                switch (tileType) {
                    case TileType::Empty:
                        break;
                    case TileType::Dust:
                        m_image.setPixel({x,y}, {200, 200, 200, 255});
                        break;
                    case TileType::Rock:
                        m_image.setPixel({x,y}, {120, 120, 120, 255});
                        break;
                    case TileType::DenseRock:
                        m_image.setPixel({x,y}, {70, 70, 70, 255});
                        break;
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

    float SamplePerlin(float x, float y) const
    {
        // Find unit grid cell containing point
        int xi = static_cast<int>(std::floor(x)) & 255;
        int yi = static_cast<int>(std::floor(y)) & 255;

        // Find relative x, y in cell
        float xf = x - std::floor(x);
        float yf = y - std::floor(y);

        // Compute fade curves
        float u = Fade(xf);
        float v = Fade(yf);

        // Hash coordinates of the 4 square corners
        int aa = m_perm[m_perm[xi] + yi];
        int ab = m_perm[m_perm[xi] + yi + 1];
        int ba = m_perm[m_perm[xi + 1] + yi];
        int bb = m_perm[m_perm[xi + 1] + yi + 1];

        // Add blended results from 4 corners
        float x1 = Lerp(Grad(aa, xf,     yf),
                        Grad(ba, xf - 1, yf), u);

        float x2 = Lerp(Grad(ab, xf,     yf - 1),
                        Grad(bb, xf - 1, yf - 1), u);

        return Lerp(x1, x2, v);
    }

    static float Fade(float t)
    {
        // 6t^5 - 15t^4 + 10t^3
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    static float Lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    static float Grad(int hash, float x, float y)
    {
        // 8 possible gradient directions
        switch (hash & 7)
        {
            case 0: return  x + y;
            case 1: return -x + y;
            case 2: return  x - y;
            case 3: return -x - y;
            case 4: return  x;
            case 5: return -x;
            case 6: return  y;
            case 7: return -y;
            default: return 0.0f;
        }
    }

    sf::Vector2u m_size;
    sf::Vector2i m_center;
    sf::Image m_image;
    sf::Texture m_texture;
    sf::Sprite* m_sprite;
    std::array<int, 512> m_perm;
};


#endif //ASTGEN_WORLD_H