// assetManager.h
#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "sound.h"


// Custom deleters to make sure smart pointers call SDL3-specific cleanup functions
struct TextureDeleter {
    void operator()(SDL_Texture* texture) const {
        if (texture) SDL_DestroyTexture(texture);
    }
};

struct FontDeleter {
    void operator()(TTF_Font* font) const {
        if (font) TTF_CloseFont(font);
    }
};

class AssetManager {
public:
    AssetManager(SDL_Renderer* renderer);
    ~AssetManager() = default;

    // Prevent copies of the manager
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Font Management
    bool LoadFont(const std::string& key, const std::string& relativePath, float fontSize);
    TTF_Font* GetFont(const std::string& key) const;
    void FreeFont(const std::string& key);

    // Texture Management
    bool LoadTexture(const std::string& key, const std::string& relativePath);
    SDL_Texture* GetTexture(const std::string& key) const;
    void FreeTexture(const std::string& key);

    // On-the-fly utility to generate text textures using a cached font.
    // Results are cached by (text, fontKey, color) and owned by the AssetManager;
    // callers must NOT SDL_DestroyTexture the returned pointer.
    SDL_Texture* CreateTextTexture(const std::string& text, const std::string& fontKey, SDL_Color color);

    // Retrieve a previously cached text texture (nullptr if not cached).
    SDL_Texture* GetTextTexture(const std::string& cacheKey) const;

    // Remove a specific cached text texture.
    void FreeTextTexture(const std::string& cacheKey);

    // Sound Management
    bool LoadSound(const std::string& key, const std::string& relativePath);
    Sound* GetSound(const std::string& key) const;
    void FreeSound(const std::string& key);

    // Wipe all cached resources
    void Clear();

private:
    std::string ResolvePath(const std::string& relativePath) const;
    std::string MakeTextCacheKey(const std::string& text, const std::string& fontKey, SDL_Color color) const;

    SDL_Renderer* m_renderer = nullptr;
    std::unordered_map<std::string, std::unique_ptr<SDL_Texture, TextureDeleter>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<TTF_Font, FontDeleter>> m_fonts;
    std::unordered_map<std::string, std::unique_ptr<SDL_Texture, TextureDeleter>> m_textTextures;

    std::unordered_map<std::string, std::unique_ptr<Sound>> m_sounds;

};