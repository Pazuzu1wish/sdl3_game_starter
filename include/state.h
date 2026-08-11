// state.h
#pragma once

#include <SDL3/SDL.h>

// Forward declare GameContext to avoid circular include issues
struct GameContext;

// ==========================================
// BASE STATE CLASS
// ==========================================
class State {
public:
    virtual ~State() = default;

    virtual void HandleEvents(GameContext* context, const SDL_Event& event) = 0;
    virtual void Update(GameContext* context, float dt) = 0;
    virtual void Render(GameContext* context, SDL_Renderer* renderer) = 0;
};

// ==========================================
// TITLE STATE
// ==========================================
class TitleState : public State {
public:
    TitleState(GameContext* context);
    ~TitleState() override;

    void HandleEvents(GameContext* context, const SDL_Event& event) override;
    void Update(GameContext* context, float dt) override;
    void Render(GameContext* context, SDL_Renderer* renderer) override;

private:
    SDL_Texture* m_textTexture = nullptr;
    SDL_FRect m_textRect = {};
    // Note: We no longer need to keep a TTF_Font* here because the AssetManager caches it!
};

// ==========================================
// GAME STATE
// ==========================================
class GameState : public State {
public:
    GameState() = default;
    ~GameState() override = default;

    void HandleEvents(GameContext* context, const SDL_Event& event) override;
    void Update(GameContext* context, float dt) override;
    void Render(GameContext* context, SDL_Renderer* renderer) override;
};

// ==========================================
// PAUSE STATE
// ==========================================
class PauseState : public State {
public:
    PauseState(GameContext* context);
    ~PauseState() override;

    void HandleEvents(GameContext* context, const SDL_Event& event) override;
    void Update(GameContext* context, float dt) override;
    void Render(GameContext* context, SDL_Renderer* renderer) override;

private:
    SDL_Texture* m_textTexture = nullptr;
    SDL_FRect m_textRect = {};
    // Note: We no longer need to keep a TTF_Font* here either!
};