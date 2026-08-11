// assetManager.cpp
#include "assetManager.h"
#include <iostream>

// Detects if SDL_image is available, otherwise falls back to standard BMPs
#if __has_include(<SDL3_image/SDL_image.h>)
#include <SDL3_image/SDL_image.h>
#define HAS_SDL_IMAGE 1
#else
#define HAS_SDL_IMAGE 0
#endif

AssetManager::AssetManager(SDL_Renderer* renderer) : m_renderer(renderer) {}

std::string AssetManager::ResolvePath(const std::string& relativePath) const {
    const char* basePath = SDL_GetBasePath();
    if (basePath) {
        return std::string(basePath) + relativePath;
    }
    return relativePath;
}

bool AssetManager::LoadFont(const std::string& key, const std::string& relativePath, float fontSize) {
    if (m_fonts.find(key) != m_fonts.end()) {
        return true;
    }

    std::string fullPath = ResolvePath(relativePath);
    TTF_Font* font = TTF_OpenFont(fullPath.c_str(), fontSize); // SDL3 uses float for size
    if (!font) {
        std::cerr << "AssetManager: Failed to load font '" << key << "' from " << fullPath << ": " << SDL_GetError() << std::endl;
        return false;
    }

    m_fonts[key] = std::unique_ptr<TTF_Font, FontDeleter>(font);
    return true;
}

TTF_Font* AssetManager::GetFont(const std::string& key) const {
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        return it->second.get();
    }
    std::cerr << "AssetManager: Font '" << key << "' was not loaded." << std::endl;
    return nullptr;
}

void AssetManager::FreeFont(const std::string& key) {
    m_fonts.erase(key);
}

bool AssetManager::LoadTexture(const std::string& key, const std::string& relativePath) {
    if (m_textures.find(key) != m_textures.end()) {
        return true; // Already loaded
    }

    std::string fullPath = ResolvePath(relativePath);
    SDL_Texture* texture = nullptr;

#if HAS_SDL_IMAGE
    texture = IMG_LoadTexture(m_renderer, fullPath.c_str());
#else
    SDL_Surface* surface = SDL_LoadBMP(fullPath.c_str());
    if (surface) {
        texture = SDL_CreateTextureFromSurface(m_renderer, surface);
        SDL_DestroySurface(surface); // SDL3 uses SDL_DestroySurface
    }
#endif

    if (!texture) {
        std::cerr << "AssetManager: Failed to load texture '" << key << "' from " << fullPath << ": " << SDL_GetError() << std::endl;
        return false;
    }

    m_textures[key] = std::unique_ptr<SDL_Texture, TextureDeleter>(texture);
    return true;
}

SDL_Texture* AssetManager::GetTexture(const std::string& key) const {
    auto it = m_textures.find(key);
    if (it != m_textures.end()) {
        return it->second.get();
    }
    std::cerr << "AssetManager: Texture '" << key << "' was not loaded." << std::endl;
    return nullptr;
}

void AssetManager::FreeTexture(const std::string& key) {
    m_textures.erase(key);
}

SDL_Texture* AssetManager::CreateTextTexture(const std::string& text, const std::string& fontKey, SDL_Color color) {
    TTF_Font* font = GetFont(fontKey);
    if (!font) return nullptr;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surface) {
        std::cerr << "AssetManager: Failed to render text surface: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_DestroySurface(surface); // Clean up CPU surface

    if (!texture) {
        std::cerr << "AssetManager: Failed to create text texture: " << SDL_GetError() << std::endl;
    }

    return texture;
}

void AssetManager::Clear() {
    m_textures.clear();
    m_fonts.clear();
}

bool AssetManager::LoadSound(const std::string& key, const std::string& relativePath) {
    if (m_sounds.find(key) != m_sounds.end()) {
        return true; // Already loaded and cached
    }

    std::string fullPath = ResolvePath(relativePath);
    auto sound = std::make_unique<Sound>();

    // SDL_LoadWAV returns a boolean in SDL3
    if (!SDL_LoadWAV(fullPath.c_str(), &sound->spec, &sound->buffer, &sound->length)) {
        std::cerr << "AssetManager: Failed to load WAV '" << key << "' from " << fullPath << ": " << SDL_GetError() << std::endl;
        return false;
    }

    m_sounds[key] = std::move(sound);
    return true;
}

Sound* AssetManager::GetSound(const std::string& key) const {
    auto it = m_sounds.find(key);
    if (it != m_sounds.end()) {
        return it->second.get();
    }
    std::cerr << "AssetManager: Sound '" << key << "' was not loaded." << std::endl;
    return nullptr;
}

void AssetManager::FreeSound(const std::string& key) {
    m_sounds.erase(key);
}