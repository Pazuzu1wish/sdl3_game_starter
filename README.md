# SDL3 Game Starter

A modern C++20 game engine template built on **SDL3** with a clean, production-ready architecture. This starter provides essential game systems (rendering, input, audio, asset management) and a state-based game flow framework.

## Features

### 🎮 Core Systems
- **State Management**: Stack-based state machine (Title → Game → Pause) with safe state transitions
- **Input Handling**: Unified keyboard + gamepad input mapping with abstract actions
- **Audio System**: Sound effects, looped music, individual volume controls with master volume
- **Asset Management**: Centralized font, texture, and sound loading with caching
- **Rendering Pipeline**: 1280×800 logical resolution with automatic letterboxing
- **Unopinionated Architecture** (2D or 3D!): Bring your own Entity Component System (ECS) or rendering pipeline. The template defaults to SDL_Renderer for quick 2D prototyping, but you can easily swap it for OpenGL, Vulkan, or the new SDL3 GPU API for full 3D games. The core engine (Input, Audio, States) doesn't care!

### 🛠️ Technical Highlights
- **Modern C++20** with RAII and smart pointers throughout
- **Memory-safe**: Uses `unique_ptr` for all resource management
- **Separation of Concerns**: Modular design with dedicated managers for input, audio, assets
- **Fullscreen Support**: Configurable window mode with logical/physical resolution handling
- **Performance-tuned**: Delta-time clamping to prevent physics instability

## Architecture

```
┌─────────────────────────────────────────┐
│            Game Loop                    │
│  HandleEvents → Update → Render         │
└──────────────┬──────────────────────────┘
               │
    ┌──────────┼──────────┐
    │          │          │
    ▼          ▼          ▼
InputManager AudioManager AssetManager
    │          │          │
    └──────────┼──────────┘
               │
            States
         (Title/Game/Pause)
```

### Key Components

| Component | Responsibility |
|-----------|-----------------|
| **Game** | Main loop, state stack management, SDL3 initialization |
| **GameContext** | Shared pointer to all managers passed to states |
| **InputManager** | Keyboard + gamepad polling, action mapping, rumble |
| **AudioManager** | SFX playback, looped music, volume control |
| **AssetManager** | Font/texture/sound loading, caching, path resolution |
| **State** | Abstract base for Title/Game/Pause states |

## Building & Running

### Prerequisites
- CMake 4.3+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2019+)
- SDL3, SDL3_ttf, SDL3_image, SDL3_shadercross via vcpkg

### Build
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run
```bash
./sdl3_game_starter
```

## Code Organization

```
sdl3_game_starter/
├── include/                    # Public headers
│   ├── game.h                  # Main game class
│   ├── gameContext.h           # Shared context struct
│   ├── inputManager.h          # Input handling
│   ├── audioManager.h          # Audio system
│   ├── assetManager.h          # Asset caching
│   ├── state.h                 # State definitions (Title/Game/Pause)
│   └── sound.h                 # Audio struct
├── src/                        # Implementation
│   ├── main.cpp               # Entry point
│   ├── game.cpp               # Core game loop
│   ├── inputManager.cpp       # Input implementation
│   ├── audioManager.cpp       # Audio implementation
│   ├── assetManager.cpp       # Asset caching logic
│   └── state.cpp              # State implementations
├── assets/                     # Game resources
│   ├── fonts/                 # TTF fonts
│   └── audio/
│       └── music/             # Background music (WAV)
└── CMakeLists.txt             # Build configuration
```

## Game States

### TitleState
- Displays "Press ENTER or X to Play" message
- Plays looped background music
- Transitions to GameState on confirm, quits on cancel

### GameState
- Main gameplay area (currently a dark blue canvas)
- Supports pause with `P` key or gamepad Start button
- Quits on cancel input

### PauseState
- Semi-transparent overlay with "PAUSED" text
- Resume with `P` key, quit with Escape/Back
- Stacks on top of GameState (doesn't destroy it)

## Input Mapping

| Action | Keyboard | Gamepad |
|--------|----------|---------|
| **Confirm** | Enter, Space | South (A) |
| **Cancel** | Escape, Backspace | East (B), Back |
| **Pause** | P | Start |
| **Up** | ↑, W | D-Pad Up |
| **Down** | ↓, S | D-Pad Down |
| **Left** | ←, A | D-Pad Left |
| **Right** | →, D | D-Pad Right |

Analog sticks and rumble are fully supported via `InputManager::GetAxis()` and `InputManager::Rumble()`.

## Audio System

### Volume Control
```cpp
audioManager->SetSFXVolume(0.8f);      // 0.0 = mute, 1.0 = max
audioManager->SetMusicVolume(0.5f);
audioManager->SetMasterVolume(0.75f);  // Multiplied with sub-volumes
```

### Playing Sounds
```cpp
// One-shot sound effect
if (assets->LoadSound("jump", "assets/audio/sfx/jump.wav")) {
    audio->PlaySFX(assets->GetSound("jump"));
}

// Looped background music
if (assets->LoadSound("music", "assets/audio/music/theme.wav")) {
    audio->PlayMusic(assets->GetSound("music"), true);
}
```

## Asset Management

All assets are cached per key, preventing duplicate loads:
```cpp
// First call loads the font
assets->LoadFont("title", "assets/fonts/font.ttf", 32.0f);

// Second call returns cached version
TTF_Font* font = assets->GetFont("title");

// Create dynamic text texture (not cached)
SDL_Texture* text = assets->CreateTextTexture("Hello", "title", {255, 255, 255, 255});
SDL_DestroyTexture(text);  // Caller owns this
```

## Development Notes

### Memory Safety
- All SDL3 resources wrapped in smart pointers with custom deleters
- State stack uses `unique_ptr` for automatic cleanup
- Audio streams destroyed immediately when playback completes
- No manual delete/free calls needed

### Performance
- Delta time clamped to 0.1s to prevent spiral-of-death during debugging
- VSync enabled by default
- Logical presentation handles resolution scaling transparently
- Asset caching prevents redundant file I/O

### Extending the Engine

**Add a new state:**
```cpp
class MenuState : public State {
    void HandleEvents(GameContext* ctx, const SDL_Event& e) override;
    void Update(GameContext* ctx, float dt) override;
    void Render(GameContext* ctx, SDL_Renderer* r) override;
};
```

**Add a new input action:**
```cpp
// In inputManager.h
enum class Action {
    // ... existing
    Attack,
    Interact
};

// In InputManager::SetupDefaultMappings()
m_keyboardMaps[Action::Attack] = { SDLK_SPACE };
m_gamepadMaps[Action::Attack] = { SDL_GAMEPAD_BUTTON_SOUTH };
```

## Known Limitations & Design Patterns

- **Looping music must be explicitly stopped**: The AudioManager requires `StopMusic()` or `StopAll()` to clean up looping streams. This is by design—non-looping SFX self-destruct when playback completes.
- **No builtin 3D rendering (can be easily added though**: SDL3 2D rendering only (use OpenGL/Vulkan integration if needed)
- **Single-threaded**: All updates happen on the main thread
- **Fullscreen enforced**: Modify `game.cpp` line 28 to enable windowed mode

**Happy game dev!** 🎮
