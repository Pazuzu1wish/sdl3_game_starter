// gameContext.h
#pragma once

// Forward declarations so the compiler knows these exist
class Game;
class AssetManager;
class InputManager;
class AudioManager;
struct SDL_Renderer;

struct GameContext {
    Game* game = nullptr;
    SDL_Renderer* renderer = nullptr;
    AssetManager* assets = nullptr;
    InputManager* input = nullptr;
    AudioManager* audio = nullptr;

    // You can easily add Camera, or Config managers here later!
};