// game.cpp
#include "game.h"
#include "../include/state.h"
#include <iostream>
#include <string>
#include <SDL3_ttf/SDL_ttf.h>


Game::Game() = default;

Game::~Game() {
    Shutdown();
}

bool Game::Init(const char* title) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!TTF_Init()) {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    m_logicalWidth = 1280;
    m_logicalHeight = 800;

    m_window = SDL_CreateWindow(title, m_logicalWidth, m_logicalHeight, SDL_WINDOW_FULLSCREEN);
    if (!m_window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // Track the actual window pixel size (logical presentation handles rendering scaling).
    SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!SDL_SetRenderLogicalPresentation(m_renderer, m_logicalWidth, m_logicalHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        std::cerr << "Warning: Failed to set logical presentation: " << SDL_GetError() << std::endl;
    }

    if (!SDL_SetRenderVSync(m_renderer, 1)) {
        std::cerr << "Warning: Could not enable VSync: " << SDL_GetError() << std::endl;
    }

    m_audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (m_audioDevice == 0) {
        std::cerr << "Warning: Failed to open default audio device: " << SDL_GetError() << std::endl;
    }

    m_assetManager = std::make_unique<AssetManager>(m_renderer);
    m_audioManager = std::make_unique<AudioManager>(m_audioDevice);

    // Font used by the FPS / debug overlay
    m_assetManager->LoadFont("DebugFont_16", "assets/fonts/font.ttf", 16.0f);


    m_context.game = this;
    m_context.renderer = m_renderer;
    m_context.assets = m_assetManager.get();
    m_context.input = &m_input;
    m_context.audio = m_audioManager.get();

    m_running = true;
    return true;
}

void Game::Run() {
    // Get high-resolution counter values
    Uint64 lastTime = SDL_GetPerformanceCounter();
    Uint64 frequency = SDL_GetPerformanceFrequency();

    // FPS / debug overlay bookkeeping
    float frameTimeAccum = 0.0f;
    int   frameCount = 0;

    while (m_running) {
        Uint64 currentTime = SDL_GetPerformanceCounter();

        // Calculate the elapsed time in seconds
        float dt = static_cast<float>(currentTime - lastTime) / frequency;
        lastTime = currentTime;

        if (dt > 0.1f) {
            dt = 0.1f;
        }

        // Update the debug overlay text once per second
        frameTimeAccum += dt;
        ++frameCount;
        if (frameTimeAccum >= 1.0f) {
            float fps  = static_cast<float>(frameCount) / frameTimeAccum;
            float avgMs = frameTimeAccum / static_cast<float>(frameCount) * 1000.0f;
            m_fpsText = "FPS: " + std::to_string(static_cast<int>(fps))
                      + "  " + std::to_string(static_cast<int>(avgMs)) + "ms";
            frameTimeAccum = 0.0f;
            frameCount = 0;
        }

        HandleEvents();
        Update(dt);

        m_audioManager->Update();

        Render();
        ApplyStateChanges();


    }
}

void Game::Update(float dt) {
    if (!m_states.empty()) {
        m_states.back()->Update(&m_context, dt);

        // Show the cursor in menus, hide it during gameplay
        SetCursorVisible(!m_states.back()->CursorHidden());
    }
}

void Game::Quit() {
    m_running = false;
}

void Game::ToggleFullscreen() {
    m_fullscreen = !m_fullscreen;
    if (!SDL_SetWindowFullscreen(m_window, m_fullscreen ? SDL_WINDOW_FULLSCREEN : 0)) {
        std::cerr << "Warning: Failed to toggle fullscreen: " << SDL_GetError() << std::endl;
    }
}

void Game::SetCursorVisible(bool visible) {
    if (m_cursorVisible == visible) return; // no-op when already in the desired state
    m_cursorVisible = visible;
    if (visible) {
        if (!SDL_ShowCursor()) {
            std::cerr << "Warning: Failed to show cursor: " << SDL_GetError() << std::endl;
        }
    } else {
        if (!SDL_HideCursor()) {
            std::cerr << "Warning: Failed to hide cursor: " << SDL_GetError() << std::endl;
        }
    }
}

// --- STATE MANAGEMENT ---

void Game::ChangeState(State* state) {
    m_stateCommands.push_back({ StateCommand::Type::Change, std::unique_ptr<State>(state) });
}

void Game::PushState(State* state) {
    m_stateCommands.push_back({ StateCommand::Type::Push, std::unique_ptr<State>(state) });
}

void Game::PopState() {
    m_stateCommands.push_back({ StateCommand::Type::Pop, nullptr });
}

void Game::ApplyStateChanges() {
    if (m_stateCommands.empty()) return;

    for (auto& cmd : m_stateCommands) {
        switch (cmd.type) {
            case StateCommand::Type::Change:
                if (!m_states.empty()) {
                    m_states.pop_back();
                }
                m_states.push_back(std::move(cmd.state));
                break;

            case StateCommand::Type::Push:
                m_states.push_back(std::move(cmd.state));
                break;

            case StateCommand::Type::Pop:
                if (!m_states.empty()) {
                    m_states.pop_back();
                }
                break;
        }
    }
    m_stateCommands.clear();
}

// --- GAME LOOP ---

void Game::HandleEvents() {
    m_input.BeginFrame();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            Quit();
        }

        // F11 toggles fullscreen / windowed mode (handled at game level, before states)
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F11) {
            ToggleFullscreen();
        }

        // React to window resizes. Logical presentation (letterbox) handles rendering
        // scaling automatically, so we only track the new window pixel size here.
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            m_windowWidth  = static_cast<int>(event.window.data1);
            m_windowHeight = static_cast<int>(event.window.data2);
            std::cout << "Window resized to " << m_windowWidth << "x" << m_windowHeight << std::endl;
        }

        m_input.ProcessEvent(event);

        if (!m_states.empty()) {
            m_states.back()->HandleEvents(&m_context, event);
        }
    }
}

void Game::Render() {
    // Each state is responsible for clearing/drawing its own background.
    // Clearing here would be redundant when a state clears itself (e.g. GameState),
    // and would wipe an overlaid state's content (e.g. PauseState over GameState).

    // Use const auto& to prevent trying to copy the unique_ptr (which is not allowed)
    for (const auto& state : m_states) {
        state->Render(&m_context, m_renderer);
    }

    // FPS / debug overlay drawn on top of every state
    RenderDebugOverlay();

    SDL_RenderPresent(m_renderer);
}

void Game::RenderDebugOverlay() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Texture* fpsTexture = m_assetManager->CreateTextTexture(m_fpsText, "DebugFont_16", white);
    if (!fpsTexture) {
        return;
    }

    float w = 0, h = 0;
    SDL_GetTextureSize(fpsTexture, &w, &h);

    // Top-left corner of the logical screen
    SDL_FRect fpsRect = { 10.0f, 10.0f, w, h };
    SDL_RenderTexture(m_renderer, fpsTexture, nullptr, &fpsRect);
}

void Game::Shutdown() {
    m_states.clear();        // RAII cleans up all states automatically
    m_stateCommands.clear(); // RAII cleans up any pending queue items

    m_audioManager.reset();
    m_assetManager.reset();

    if (m_audioDevice) {
        SDL_CloseAudioDevice(m_audioDevice);
        m_audioDevice = 0;
    }

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    TTF_Quit();
    SDL_Quit();
}