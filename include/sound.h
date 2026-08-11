// sound.h
#pragma once
#include <SDL3/SDL.h>

struct Sound {
    SDL_AudioSpec spec;
    Uint8* buffer = nullptr;
    Uint32 length = 0;

    ~Sound() {
        if (buffer) {
            // Note: In SDL3, SDL_FreeWAV is removed. We use standard SDL_free
            SDL_free(buffer);
        }
    }
};