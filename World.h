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

    Tile GetTile(int x, int y) {
        Tile toReturn;
        float dx = static_cast<float>(x - m_center.x);
        float dy = static_cast<float>(y - m_center.y);

        float distance = std::sqrt(dx * dx + dy * dy);

        float ringNoiseScale = 0.02f;
        float octave0 = SamplePerlin(x * ringNoiseScale * 5.f, y * ringNoiseScale * 5.f);
        float octave1 = SamplePerlin(x * ringNoiseScale, y * ringNoiseScale);
        float octave2 = 2 * SamplePerlin(x * ringNoiseScale * 0.15f, y * ringNoiseScale * 0.15f);
        float octave3 = 4 * SamplePerlin(x * ringNoiseScale * 0.03f, y * ringNoiseScale * 0.03f);
        float ringNoise    = octave1 + octave2 + octave3;
        float caveNoise = (3.f + octave1 + octave2) / 6.f;
        float oreNoise = octave0;
        float nx = 0.007f * (0.8f * x - 0.6f * y);
        float ny = 0.007f * (0.6f * x + 0.8f * y);
        float tunnelNoise = SamplePerlin(nx + 500.0f, ny + 900.0f);

        float surfaceRadiusBase = 3900.f;
        float surfaceRadiusAmplitude = 20.f;
        float surfaceRadius = surfaceRadiusBase + ringNoise * surfaceRadiusAmplitude;
        if (distance > surfaceRadius) {
            toReturn.tileType = TileType::Empty;
            toReturn.oreType = OreType::EmptyOre;
            return toReturn;
        }

        float outerMantleRadiusBase = 3000.f;
        float outerMantleAmplitude = 100.f;
        float outerMantleRadius = outerMantleRadiusBase + ringNoise * outerMantleAmplitude;
        if (distance > outerMantleRadius) {
            if (std::abs(tunnelNoise) < 0.1f) {
                toReturn.tileType = TileType::Empty;
            }
            else if (caveNoise > 0.65f) {
                toReturn.tileType = TileType::Empty;
            }
            else {
                toReturn.tileType = TileType::Dust;
            }
            if (oreNoise > 0.4f) {
                toReturn.oreType = OreType::DustOre;
            }
            else if (oreNoise < -0.7f) {
                toReturn.oreType = OreType::RockOre;
            }
            return toReturn;
        }

        float innerMantleRadiusBase = 2000.f;
        float innerMantleAmplitude = 50.f;
        float innerMantleRadius = innerMantleRadiusBase + ringNoise * innerMantleAmplitude;
        if (distance > innerMantleRadius) {
            if (caveNoise > 0.6f) {
                toReturn.tileType = TileType::Empty;
            }
            else {
                toReturn.tileType = TileType::Rock;
            }
            if (oreNoise > 0.4f) {
                toReturn.oreType = OreType::RockOre;
            }
            if (oreNoise < -0.6f) {
                toReturn.oreType = OreType::DenseRockOre;
            }
            return toReturn;
        }

        float coreRadiusBase = 1000.f;
        float coreRadiusAmplitude = 50.f;
        float coreRadius = coreRadiusBase + ringNoise * coreRadiusAmplitude;
        if (distance > coreRadius) {
            toReturn.tileType = TileType::DenseRock;
            if (oreNoise > 0.4f) {
                toReturn.oreType = OreType::DenseRockOre;
            }
            return toReturn;
        }

        toReturn.tileType = TileType::SuperDenseRock;
        if (oreNoise > 0.4f) {
            toReturn.oreType = OreType::SuperDenseRockOre;
        }
        return toReturn;

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
                auto [tileType, oreType] = GetTile(x, y);
                switch (tileType) {
                    case TileType::Empty:
                        break;
                    case TileType::Dust:
                        if (oreType != OreType::EmptyOre) {
                            if (oreType != OreType::DustOre) {
                                m_image.setPixel({x,y}, {164, 81, 255, 255});
                            }
                            else {
                                m_image.setPixel({x,y}, {255, 118, 125, 255});
                            }
                        }
                        else {
                            m_image.setPixel({x,y}, {171, 187, 207, 255});
                        }
                        break;
                    case TileType::Rock:
                        if (oreType != OreType::EmptyOre) {
                            if (oreType != OreType::RockOre) {
                                m_image.setPixel({x,y}, {98, 122, 102, 255});
                            }
                            else {
                                m_image.setPixel({x,y}, {168, 122, 199, 255});
                            }
                        }
                        else {
                            m_image.setPixel({x,y}, {133, 149, 207, 255});
                        }
                        break;
                    case TileType::DenseRock:
                        if (oreType != OreType::EmptyOre) {
                            m_image.setPixel({x,y}, {205, 70, 250, 255});
                        }
                        else {
                            m_image.setPixel({x,y}, {96, 112, 133, 255});
                        }
                        break;
                    case TileType::SuperDenseRock:
                        if (oreType != OreType::EmptyOre) {
                            m_image.setPixel({x,y}, {76, 217, 99, 255});
                        }
                        else {
                            m_image.setPixel({x,y}, {73, 88, 107, 255});
                        }
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