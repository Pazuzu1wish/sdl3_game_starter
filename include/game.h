// game.h
#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include "inputManager.h"
#include "assetManager.h"
#include "gameContext.h"
#include "audioManager.h"


class State;

class Game {
public:
    Game();
    ~Game();

    bool Init(const char* title);
    void Run();
    void Shutdown();
    void Quit();

    void ChangeState(State* state);
    void PushState(State* state);
    void PopState();

    // Context Accessor
    GameContext* GetContext() { return &m_context; }

    int GetScreenWidth()  const    { return m_logicalWidth; }
    int GetScreenHeight() const    { return m_logicalHeight; }

private:
    void HandleEvents();
    void Update(float dt);
    void Render();
    void ApplyStateChanges();

    SDL_Window*    m_window   = nullptr;
    SDL_Renderer*  m_renderer = nullptr;
    bool           m_running  = false;

    //int m_screenWidth  = 0;
    //int m_screenHeight = 0;
    int m_logicalWidth = 1280;
    int m_logicalHeight = 800;

    SDL_AudioDeviceID m_audioDevice = 0;
    std::unique_ptr<AudioManager> m_audioManager;

    InputManager m_input;
    std::unique_ptr<AssetManager> m_assetManager;

    // The shared context that will be passed to states
    GameContext m_context;

    std::vector<std::unique_ptr<State>> m_states;

    struct StateCommand {
        enum class Type { Push, Pop, Change } type;
        std::unique_ptr<State> state = nullptr; // Let smart pointer manage lifecycle
    };
    std::vector<StateCommand> m_stateCommands;
};