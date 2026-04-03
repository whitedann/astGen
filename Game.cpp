//
// Created by Daniel on 3/17/2026.
//

#include "Game.h"

Game::Game() {
    SetupClasses();
}

void Game::Update(float l_dT) const {
    m_camera->Update(*m_renderWindow);
    m_chunkManager->Update(l_dT);
    m_map->Update(l_dT);
}

void Game::Draw() const {
    m_renderWindow->clear();
    m_map->Draw(m_renderWindow.get());
    m_chunkManager->Draw(m_renderWindow.get());
    m_renderWindow->display();
}

void Game::HandleEvents() const {
    while (auto event = m_renderWindow->pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            m_renderWindow->close();
        if (const auto key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::P) {
                m_renderWindow->close();
            }
        }
        m_camera->HandleEvent(*m_renderWindow, *event);
    }

    sf::Vector2i mousePixel = sf::Mouse::getPosition(*m_renderWindow);
    sf::Vector2f mouseWorld = m_renderWindow->mapPixelToCoords(mousePixel);

    auto isSolid = m_map->GetSolidMask(mouseWorld);
    m_chunkManager->SetText6(isSolid ? std::string("Solid") : std::string("Normal"));

    int tx = static_cast<int>(std::floor(mousePixel.x));
    int ty = static_cast<int>(std::floor(mousePixel.y));
    m_chunkManager->SetText5(std::to_string(tx) + ", " + std::to_string(ty));
}

void Game::SetupClasses() {
    sf::Vector2u worldSize = {WORLD_SIZE_CHUNKS * CHUNK_SIZE_PX,WORLD_SIZE_CHUNKS * CHUNK_SIZE_PX};
    sf::Vector2u windowSize = {1000,1000};

    sf::Vector2i playerStartTile = {static_cast<int>(3 * worldSize.x / 4), static_cast<int>(3 * worldSize.y / 4)};
    m_renderWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode(windowSize), "Window");
    m_world = std::make_unique<World>(worldSize);
    m_map = std::make_unique<Map>(m_world.get());
    m_camera = std::make_unique<Camera>(windowSize, static_cast<sf::Vector2f>(playerStartTile));
    m_chunkManager = std::make_unique<ChunkManager>(m_renderWindow.get());
    m_chunkManager->AddChunkLoader(m_map.get());
}
