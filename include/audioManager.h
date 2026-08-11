// audioManager.h
#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include "sound.h"

class AudioManager {
public:
    AudioManager(SDL_AudioDeviceID deviceID);
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // Fire-and-forget sound effect
    void PlaySFX(Sound* sound);

    // Play loopable background track
    void PlayMusic(Sound* sound, bool loop = true);
    void StopMusic();

    // Must be called once per frame to clean up inactive streams and handle looping
    void Update();

    // Volume controls (ranges from 0.0f [mute] to 1.0f [max])
    void SetSFXVolume(float volume);
    void SetMusicVolume(float volume);
    void SetMasterVolume(float volume);

    float GetSFXVolume() const { return m_sfxVolume; }
    float GetMusicVolume() const { return m_musicVolume; }
    float GetMasterVolume() const { return m_masterVolume; }

    // Debug: Get active stream counts
    void GetStreamCount(int& activeCount, int& loopingCount) const;

    void StopAll();

private:
    SDL_AudioDeviceID m_deviceID = 0;

    struct ActiveStream {
        SDL_AudioStream* stream = nullptr;
        Sound* asset = nullptr; // Track original asset for loop re-feeding
        bool isMusic = false;
        bool isLooping = false;
    };

    std::vector<ActiveStream> m_activeStreams;

    float m_sfxVolume = 0.8f;
    float m_musicVolume = 0.5f;
    float m_masterVolume = 1.0f;
};
