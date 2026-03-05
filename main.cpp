#include <iostream>
#include <SFML/Graphics.hpp>

#include "Camera.h"
#include "World.h"
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {

    sf::Vector2u worldSize = {12000,12000};
    sf::Vector2u windowSize = {900,900};

    sf::Vector2u playerStartTile = {3 * worldSize.x / 4, 3 * worldSize.y / 4};

    sf::RenderWindow window(
        sf::VideoMode(windowSize),
        "Window");

    World world(worldSize);

    Camera camera(
        worldSize,
        windowSize,
        static_cast<sf::Vector2f>(playerStartTile)
    );

    sf::CircleShape box;
    box.setRadius(4.f);
    box.setFillColor(sf::Color::Yellow);

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

        camera.Update(window);
        camera.Apply(window);

        box.setPosition(camera.GetZoomCenter());

        window.clear();
        world.Draw(window);
        window.draw(box);

        window.display();
    }
    return 0;
}