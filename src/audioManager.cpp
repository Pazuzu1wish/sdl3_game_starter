// audioManager.cpp
#include "audioManager.h"
#include "sound.h"
#include <algorithm>
#include <iostream>

AudioManager::AudioManager(SDL_AudioDeviceID deviceID) : m_deviceID(deviceID) {}

AudioManager::~AudioManager() {
    StopAll();
}

void AudioManager::PlaySFX(Sound* sound) {
    if (!sound || m_deviceID == 0) return;

    // Create an audio stream mapping the WAV spec to device output format
    SDL_AudioStream* stream = SDL_CreateAudioStream(&sound->spec, nullptr);
    if (!stream) {
        std::cerr << "AudioManager: Failed to create SFX stream: " << SDL_GetError() << std::endl;
        return;
    }

    // Bind our stream to the logical playback device
    if (!SDL_BindAudioStream(m_deviceID, stream)) {
        std::cerr << "AudioManager: Failed to bind SFX stream: " << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(stream);
        return;
    }

    // Adjust volume (gain) based on Master & SFX volumes
    float volume = m_sfxVolume * m_masterVolume;
    SDL_SetAudioStreamGain(stream, volume);

    // Queue raw PCM buffer to the stream and flush it
    SDL_PutAudioStreamData(stream, sound->buffer, sound->length);
    SDL_FlushAudioStream(stream);

    m_activeStreams.push_back({ stream, sound, false, false });
}

void AudioManager::PlayMusic(Sound* sound, bool loop) {
    if (!sound || m_deviceID == 0) return;

    StopMusic(); // Halt currently playing track first

    SDL_AudioStream* stream = SDL_CreateAudioStream(&sound->spec, nullptr);
    if (!stream) {
        std::cerr << "AudioManager: Failed to create music stream: " << SDL_GetError() << std::endl;
        return;
    }

    if (!SDL_BindAudioStream(m_deviceID, stream)) {
        std::cerr << "AudioManager: Failed to bind music stream: " << SDL_GetError() << std::endl;
        SDL_DestroyAudioStream(stream);
        return;
    }

    float volume = m_musicVolume * m_masterVolume;
    SDL_SetAudioStreamGain(stream, volume);

    SDL_PutAudioStreamData(stream, sound->buffer, sound->length);

    if (!loop) {
        SDL_FlushAudioStream(stream);
    }

    m_activeStreams.push_back({ stream, sound, true, loop });
}

void AudioManager::Update() {
    for (auto it = m_activeStreams.begin(); it != m_activeStreams.end();) {
        // Handle background music looping
        if (it->isMusic && it->isLooping && it->asset) {
            if (SDL_GetAudioStreamQueued(it->stream) < static_cast<int>(it->asset->length)) {
                SDL_PutAudioStreamData(it->stream, it->asset->buffer, it->asset->length);
            }
            ++it;
            continue;
        }

        // If the stream has run empty, its playback is complete
        if (SDL_GetAudioStreamQueued(it->stream) == 0) {
            SDL_DestroyAudioStream(it->stream);
            it = m_activeStreams.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioManager::SetSFXVolume(float volume) {
    m_sfxVolume = std::clamp(volume, 0.0f, 1.0f);
    for (auto& active : m_activeStreams) {
        if (!active.isMusic) {
            SDL_SetAudioStreamGain(active.stream, m_sfxVolume * m_masterVolume);
        }
    }
}

void AudioManager::SetMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.0f, 1.0f);
    for (auto& active : m_activeStreams) {
        if (active.isMusic) {
            SDL_SetAudioStreamGain(active.stream, m_musicVolume * m_masterVolume);
        }
    }
}

void AudioManager::SetMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    for (auto& active : m_activeStreams) {
        float subVolume = active.isMusic ? m_musicVolume : m_sfxVolume;
        SDL_SetAudioStreamGain(active.stream, subVolume * m_masterVolume);
    }
}

void AudioManager::StopMusic() {
    for (auto it = m_activeStreams.begin(); it != m_activeStreams.end();) {
        if (it->isMusic) {
            SDL_DestroyAudioStream(it->stream);
            it = m_activeStreams.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioManager::StopAll() {
    for (auto& active : m_activeStreams) {
        SDL_DestroyAudioStream(active.stream);
    }
    m_activeStreams.clear();
}