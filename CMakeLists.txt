cmake_minimum_required(VERSION 4.1)
project(astGen)
find_package(SFML CONFIG REQUIRED
        COMPONENTS
        Graphics
        Window
        System
)

set(CMAKE_CXX_STANDARD 20)

add_executable(astGen main.cpp
        World.h
        Camera.h
        TileTypes.h
        ChunkManager/Chunk.h
        ChunkManager/ChunkManager.h
        ChunkManager/ChunkManager.cpp
        ChunkManager/ChunkEnums.h
        MapManager/Map.h
        MapManager/Map.cpp
        ChunkManager/ChunkLoader.h)

target_link_libraries(astGen
        PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)