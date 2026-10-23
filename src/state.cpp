// state.cpp
#include "state.h"
#include "game.h"
#include <cmath>
#include <iostream>

// ==========================================
// TITLE STATE
// ==========================================

TitleState::TitleState(GameContext* context) {
    AssetManager* assets = context->assets;
    Game* game = context->game;

    // Load and cache the menu font at size 32
    assets->LoadFont("MenuFont_32", "assets/fonts/font.ttf", 32.0f);

    SDL_Color white = {255, 255, 255, 255};
    m_textTexture = assets->CreateTextTexture("Press ENTER or X to Play", "MenuFont_32", white);

    if (m_textTexture) {
        float w = 0, h = 0;
        SDL_GetTextureSize(m_textTexture, &w, &h);

        m_textRect.w = w;
        m_textRect.h = h;
        m_textRect.x = game->GetScreenWidth()  / 2.0f - (w / 2.0f);
        m_textRect.y = game->GetScreenHeight() / 2.0f - (h / 2.0f);
    }

    if (assets->LoadSound("TitleTheme", "assets/audio/music/Rust-Orbit.wav")) {
        Sound* music = assets->GetSound("TitleTheme");
        if (music) {
            context->audio->PlayMusic(music, true); // 'true' tells AudioManager to loop it
        }
    }
}

TitleState::~TitleState() {
    // Only destroy the dynamic text texture this state specifically created
    if (m_textTexture) {
        SDL_DestroyTexture(m_textTexture);
    }
}

void TitleState::HandleEvents(GameContext* context, const SDL_Event& event) {
    // Left empty since we now poll actions inside Update()
}

void TitleState::Update(GameContext* context, float dt) {
    InputManager* input = context->input;
    Game* game = context->game;

    if (input->IsPressed(Action::Confirm)) {

        context->audio->StopMusic();

        game->ChangeState(new GameState(context));
    }

    if (input->IsPressed(Action::Cancel)) {
        game->Quit();
    }
}

void TitleState::Render(GameContext* context, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (m_textTexture) {
        SDL_RenderTexture(renderer, m_textTexture, nullptr, &m_textRect);
    }
}

// ==========================================
// GAME STATE
// ==========================================

GameState::GameState(GameContext* context) {
    // Load the font if it isn't loaded already
    context->assets->LoadFont("GameFont_32", "assets/fonts/font.ttf", 32.0f);
}

GameState::~GameState() {
    if (m_textTexture) {
        SDL_DestroyTexture(m_textTexture);
    }
}

void GameState::HandleEvents(GameContext* context, const SDL_Event& event) {
    // Left empty
}

void GameState::Update(GameContext* context, float dt) {
    InputManager* input = context->input;
    Game* game = context->game;

    m_time += dt;

    // --- Typewriter Effect Logic ---
    float typingSpeed = 10.0f; // 10 characters per second
    float pauseDuration = 1.5f; // Wait 1.5 seconds after full text is shown
    float loopDuration = (m_fullText.length() / typingSpeed) + pauseDuration;

    // Use modulo to loop the local time
    float localTime = std::fmod(m_time, loopDuration);

    int newVisibleChars = static_cast<int>(localTime * typingSpeed);
    if (newVisibleChars > m_fullText.length()) {
        newVisibleChars = m_fullText.length();
    }

    // Only recreate the texture if the number of visible characters has changed
    if (newVisibleChars != m_visibleChars) {
        m_visibleChars = newVisibleChars;

        if (m_textTexture) {
            SDL_DestroyTexture(m_textTexture);
            m_textTexture = nullptr;
        }

        if (m_visibleChars > 0) {
            std::string currentText = m_fullText.substr(0, m_visibleChars);
            SDL_Color white = {255, 255, 255, 255};
            m_textTexture = context->assets->CreateTextTexture(currentText, "GameFont_32", white);

            if (m_textTexture) {
                float w = 0, h = 0;
                SDL_GetTextureSize(m_textTexture, &w, &h);

                m_textRect.w = w;
                m_textRect.h = h;
                m_textRect.x = game->GetScreenWidth() / 2.0f - (w / 2.0f);
                m_textRect.y = game->GetScreenHeight() / 3.0f; // Put it a bit above the center
            }
        }
    }
    // -------------------------------

    if (input->IsPressed(Action::Pause)) {
        game->PushState(new PauseState(context));
    }

    if (input->IsPressed(Action::Cancel)) {
        game->Quit();
    }
}

void GameState::Render(GameContext* context, SDL_Renderer* renderer) {
    // Color morphing background
    Uint8 r = static_cast<Uint8>(30 + 20 * std::sin(m_time * 1.0f));
    Uint8 g = static_cast<Uint8>(30 + 20 * std::sin(m_time * 0.8f + 2.0f));
    Uint8 b = static_cast<Uint8>(60 + 30 * std::sin(m_time * 0.5f + 4.0f));

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);

    // Draw the typewriter text
    if (m_textTexture) {
        SDL_RenderTexture(renderer, m_textTexture, nullptr, &m_textRect);
    }
}

// ==========================================
// PAUSE STATE
// ==========================================

PauseState::PauseState(GameContext* context) {
    AssetManager* assets = context->assets;
    Game* game = context->game;

    // Load and cache the menu font at size 48 (different size is stored separately)
    assets->LoadFont("MenuFont_48", "assets/fonts/font.ttf", 48.0f);

    SDL_Color red = {255, 0, 0, 255};
    m_textTexture = assets->CreateTextTexture("PAUSED", "MenuFont_48", red);

    if (m_textTexture) {
        float w = 0, h = 0;
        SDL_GetTextureSize(m_textTexture, &w, &h);

        m_textRect.w = w;
        m_textRect.h = h;
        m_textRect.x = game->GetScreenWidth()  / 2.0f - (w / 2.0f);
        m_textRect.y = game->GetScreenHeight() / 2.0f - (h / 2.0f);
    }
}

PauseState::~PauseState() {
    if (m_textTexture) {
        SDL_DestroyTexture(m_textTexture);
    }
}

void PauseState::HandleEvents(GameContext* context, const SDL_Event& event) {
    // Left empty
}

void PauseState::Update(GameContext* context, float dt) {
    InputManager* input = context->input;
    Game* game = context->game;

    // Pressing 'P' or 'Start' unpauses the game
    if (input->IsPressed(Action::Pause)) {
        game->PopState();
    }

    // Pressing 'Escape' or Gamepad Back/East quits
    if (input->IsPressed(Action::Cancel)) {
        game->Quit();
    }
}

void PauseState::Render(GameContext* context, SDL_Renderer* renderer) {
    // Semi-transparent overlay over the previous GameState render
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);

    Game* game = context->game;
    SDL_FRect screenRect = { 0, 0, (float)game->GetScreenWidth(), (float)game->GetScreenHeight() };
    SDL_RenderFillRect(renderer, &screenRect);

    // Centered PAUSED text
    if (m_textTexture) {
        SDL_RenderTexture(renderer, m_textTexture, nullptr, &m_textRect);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}