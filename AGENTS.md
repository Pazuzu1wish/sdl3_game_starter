# AGENTS.md — SDL3 Game Starter

Compact instructions for agents working in this repo. Everything here is verified against `CMakeLists.txt`, `README.md`, and source; nothing generic.

## Repo shape
- Single CMake project, one executable target: `sdl3_game_starter`
- C++20 (`CMAKE_CXX_STANDARD 20`)
- Entry point: `src/main.cpp` → `Game::Init(...)` → `game.Run()` (state stack starts at `TitleState`)
- Headers: `include/`; implementations: `src/`; assets: `assets/` (fonts, audio/music)
- CLion build directories `cmake-build-debug/` and `cmake-build-release/` are gitignored — do not commit them

## Dependencies (via vcpkg)
SDL3, SDL3_ttf, SDL3_image, nlohmann_json, glm, EnTT. (`SDL3_shadercross` was removed — do not re-add.) CMake uses `find_package(... CONFIG REQUIRED)`, so vcpkg must be installed and on `CMAKE_PREFIX_PATH`/toolchain, or packages installed system-wide. No `vcpkg.json` exists in the repo.

## Build
```bash
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```
Custom target `sync_assets` copies `assets/` into `<build-dir>/assets` and is a dependency of the executable, so assets are synchronized automatically on build. Assets load via paths relative to the working directory.

## Run
Run the binary **from its build directory** so relative asset paths resolve:
```bash
cd cmake-build-debug && ./sdl3_game_starter
```
Running from repo root will fail to find assets.

## Code conventions / gotchas
- Game states: subclass `State`, implement `HandleEvents`/`Update`/`Render`, push with `game.PushState(new MyState(game.GetContext()))`.
- Input actions live in `inputManager.h` `Action` enum; wire defaults in `InputManager::SetupDefaultMappings()`.
- Assets are cached per key (`LoadFont`/`LoadSound`/`GetFont`/`GetSound`). Textures from `CreateTextTexture` are **caller-owned** — call `SDL_DestroyTexture` yourself.
- Looping music must be explicitly stopped (`StopMusic()`/`StopAll()`); non-looping SFX self-destruct when playback ends.
- Fullscreen is enforced by default; see `game.cpp` around line 28 to switch to windowed.
- Delta time is clamped to 0.1s; VSync enabled by default.

## Verification
There is no test suite, no linter, no formatter, and no CI config in this repo. The only verification is building and running the executable. If you change CMakeLists dependency logic, reconfigure CMake (find_package calls re-run).

## Excluded (not present in repo)
No `AGENTS.md` predecessor to reconcile, no `.github` workflows, no pre-commit config, no `opencode.json`, no codegen/migrations.