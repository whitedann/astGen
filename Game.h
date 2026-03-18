//
// Created by Daniel on 3/17/2026.
//

#ifndef ASTGEN_GAME_H
#define ASTGEN_GAME_H
#include <memory>

#include "Camera.h"
#include "World.h"
#include "ChunkManager/ChunkManager.h"
#include "MapManager/Map.h"

class Game {

public:

    Game();
    void Update(float l_dT) const;
    void Draw() const;
    void HandleEvents() const;

    sf::RenderWindow* GetWindow() const {
        return m_renderWindow.get();
    }

    Camera* GetCamera() const {
        return m_camera.get();
    }

private:

    void SetupClasses();

    std::unique_ptr<Map> m_map;
    std::unique_ptr<ChunkManager> m_chunkManager;
    std::unique_ptr<sf::RenderWindow> m_renderWindow;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<World> m_world;
};

#endif //ASTGEN_GAME_H