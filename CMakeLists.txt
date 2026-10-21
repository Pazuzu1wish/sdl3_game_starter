cmake_minimum_required(VERSION 4.3)
project(sdl3_game_starter)

set(CMAKE_CXX_STANDARD 20)

add_executable(sdl3_game_starter src/main.cpp
        src/assetManager.cpp
        src/audioManager.cpp
        src/game.cpp
        include/game.h
        src/inputManager.cpp
        include/inputManager.h
        include/gameContext.h
        src/state.cpp
        include/state.h
        include/sound.h
)

add_custom_command(TARGET sdl3_game_starter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:sdl3_game_starter>/assets"
        COMMENT "Copying assets to executable directory..."
)

target_include_directories(sdl3_game_starter PRIVATE ${CMAKE_SOURCE_DIR}/include)

find_package(nlohmann_json CONFIG REQUIRED)

find_package(SDL3 CONFIG REQUIRED)

find_package(SDL3_ttf CONFIG REQUIRED)

find_package(glm CONFIG REQUIRED)

find_package(EnTT CONFIG REQUIRED)

find_package(SDL3_image CONFIG REQUIRED)

target_link_libraries(sdl3_game_starter PRIVATE SDL3::SDL3  SDL3_ttf::SDL3_ttf  glm::glm
        nlohmann_json::nlohmann_json  EnTT::EnTT  SDL3_image::SDL3_image)
