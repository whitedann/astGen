#include <iostream>
#include <SFML/Graphics.hpp>

#include "Camera.h"
#include "Game.h"
#include "World.h"
#include "ChunkManager/Chunk.h"
#include "ChunkManager/ChunkManager.h"
#include "MapManager/Map.h"

int main() {

    Game game;
    auto window = game.GetWindow();

    sf::Clock clock;
    sf::Time currentTime = clock.getElapsedTime();

    while (window->isOpen())
    {
        sf::Time newTime = clock.getElapsedTime();
        sf::Time frameTime = newTime - currentTime;
        if (frameTime.asSeconds() > 0.25f) {
            frameTime = sf::Time(sf::seconds(0.25f));
        }
        game.HandleEvents();
        game.Update(frameTime.asSeconds());
        game.Draw();
    }

    return 0;
}
