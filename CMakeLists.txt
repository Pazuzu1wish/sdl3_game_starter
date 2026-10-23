cmake_minimum_required(VERSION 4.3)
project(sdl3_game_starter)

set(CMAKE_CXX_STANDARD 20)

add_executable(sdl3_game_starter
        src/main.cpp
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

# ---------------------------------------------------------------------------
# Incremental Asset Sync & Prune
# ---------------------------------------------------------------------------
# Generate a sync script in the build directory during configure time
set(SYNC_ASSETS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/sync_assets.cmake")
file(WRITE "${SYNC_ASSETS_SCRIPT}" [=[
if(NOT EXISTS "${SRC_DIR}")
    return()
endif()

if(EXISTS "${DEST_DIR}")
    # 1. Prune stale files no longer present in source
    file(GLOB_RECURSE DEST_FILES RELATIVE "${DEST_DIR}" "${DEST_DIR}/*")
    foreach(FILE IN LISTS DEST_FILES)
        if(NOT EXISTS "${SRC_DIR}/${FILE}")
            message(STATUS "Pruning deleted asset: ${FILE}")
            file(REMOVE "${DEST_DIR}/${FILE}")
        endif()
    endforeach()

    # 2. Prune empty / stale subdirectories
    file(GLOB_RECURSE DEST_DIRS LIST_DIRECTORIES true RELATIVE "${DEST_DIR}" "${DEST_DIR}/*")
    foreach(DIR IN LISTS DEST_DIRS)
        if(IS_DIRECTORY "${DEST_DIR}/${DIR}" AND NOT EXISTS "${SRC_DIR}/${DIR}")
            file(REMOVE_RECURSE "${DEST_DIR}/${DIR}")
        endif()
    endforeach()
endif()

# 3. Incrementally copy only new or modified files
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_directory_if_different "${SRC_DIR}" "${DEST_DIR}"
    RESULT_VARIABLE RES
)
if(NOT RES EQUAL 0)
    message(FATAL_ERROR "Asset synchronization failed.")
endif()
]=])

# Custom target runs every build check; takes ~1-2ms if nothing changed
add_custom_target(sync_assets
        COMMAND ${CMAKE_COMMAND}
        -DSRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
        -DDEST_DIR="$<TARGET_FILE_DIR:sdl3_game_starter>/assets"
        -P "${SYNC_ASSETS_SCRIPT}"
        COMMENT "Synchronizing assets (incremental + pruning)..."
)

# Ensure assets are up-to-date before running the game
add_dependencies(sdl3_game_starter sync_assets)
# ---------------------------------------------------------------------------

target_include_directories(sdl3_game_starter PRIVATE ${CMAKE_SOURCE_DIR}/include)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

target_link_libraries(sdl3_game_starter PRIVATE
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
        glm::glm
        nlohmann_json::nlohmann_json
        EnTT::EnTT
        SDL3_image::SDL3_image
)