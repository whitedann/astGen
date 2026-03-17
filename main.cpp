#include <iostream>
#include <SFML/Graphics.hpp>

#include "Camera.h"
#include "World.h"
#include "ChunkManager/Chunk.h"
#include "ChunkManager/ChunkManager.h"
#include "MapManager/Map.h"

int main() {

    sf::Vector2u worldSize = {9000,9000};
    sf::Vector2u windowSize = {1000,1000};

    sf::Vector2i playerStartTile = {static_cast<int>(3 * worldSize.x / 4), static_cast<int>(3 * worldSize.y / 4)};

    sf::RenderWindow window(
        sf::VideoMode(windowSize),
        "Window");

    World world(worldSize);
    std::unique_ptr<Map> map = std::make_unique<Map>();
    ChunkManager chunkManager(window.getView());
    chunkManager.AddChunkLoader(map.get());


    Camera camera(
        windowSize,
        static_cast<sf::Vector2f>(playerStartTile)
    );

    sf::CircleShape box;
    box.setRadius(1.f);
    box.setFillColor(sf::Color::Red);

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (auto key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::P) {
                    window.close();
                }
            }
            camera.HandleEvent(window, *event);
        }

        chunkManager.Update(0.f);
        camera.Update(window);
        box.setPosition(camera.GetZoomCenter());

        window.clear();
        world.Draw(window);
        window.draw(box);

        window.display();
    }
    return 0;
}
