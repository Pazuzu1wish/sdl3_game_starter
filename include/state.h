// state.h
#pragma once

#include <string>
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
};

// ==========================================
// GAME STATE
// ==========================================

class GameState : public State {
public:
    GameState(GameContext* context);
    ~GameState() override;

    void HandleEvents(GameContext* context, const SDL_Event& event) override;
    void Update(GameContext* context, float dt) override;
    void Render(GameContext* context, SDL_Renderer* renderer) override;

private:
    float m_time = 0.0f;

    // Typewriter effect variables
    std::string m_fullText = "GAMEPLAY STATE";
    int m_visibleChars = 0;
    SDL_Texture* m_textTexture = nullptr;
    SDL_FRect m_textRect = {};
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
};