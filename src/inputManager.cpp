// inputManager.cpp
#include "inputManager.h"
#include <cmath>
#include <iostream>

InputManager::InputManager() {
    SDL_InitSubSystem(SDL_INIT_GAMEPAD);

    // Check for already-connected gamepads on startup
    int count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
    if (gamepads && count > 0) {
        ConnectGamepad(gamepads[0]);
    }
    SDL_free(gamepads);

    SetupDefaultMappings();
}

InputManager::~InputManager() {
    DisconnectGamepad();
}

void InputManager::SetupDefaultMappings() {
    // Map multiple inputs to a single abstract Action
    m_keyboardMaps[Action::Confirm] = { SDLK_RETURN, SDLK_SPACE };
    m_gamepadMaps[Action::Confirm] = { SDL_GAMEPAD_BUTTON_SOUTH };

    m_keyboardMaps[Action::Cancel] = { SDLK_ESCAPE, SDLK_BACKSPACE };
    m_gamepadMaps[Action::Cancel] = { SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_BUTTON_BACK };

    m_keyboardMaps[Action::Pause] = { SDLK_P };
    m_gamepadMaps[Action::Pause] = { SDL_GAMEPAD_BUTTON_START };

    m_keyboardMaps[Action::Up] = { SDLK_UP, SDLK_W };
    m_gamepadMaps[Action::Up] = { SDL_GAMEPAD_BUTTON_DPAD_UP };

    m_keyboardMaps[Action::Down] = { SDLK_DOWN, SDLK_S };
    m_gamepadMaps[Action::Down] = { SDL_GAMEPAD_BUTTON_DPAD_DOWN };

    m_keyboardMaps[Action::Left] = { SDLK_LEFT, SDLK_A };
    m_gamepadMaps[Action::Left] = { SDL_GAMEPAD_BUTTON_DPAD_LEFT };

    m_keyboardMaps[Action::Right] = { SDLK_RIGHT, SDLK_D };
    m_gamepadMaps[Action::Right] = { SDL_GAMEPAD_BUTTON_DPAD_RIGHT };
}

void InputManager::BeginFrame() {
    // Keep a snapshot of the last frame's keyboard and gamepad states
    m_previousKeys = m_currentKeys;

    for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++) {
        m_previousGamepadButtons[i] = m_currentGamepadButtons[i];
    }
}

void InputManager::ProcessEvent(const SDL_Event& event) {
    // 1. Process Keyboard events
    if (event.type == SDL_EVENT_KEY_DOWN) {
        m_currentKeys[event.key.key] = true;
    }
    else if (event.type == SDL_EVENT_KEY_UP) {
        m_currentKeys[event.key.key] = false;
    }

    // 2. Process Gamepad events
    switch (event.type) {
        case SDL_EVENT_GAMEPAD_ADDED:
            if (!m_gamepad) ConnectGamepad(event.gdevice.which);
            break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            if (event.gdevice.which == m_instanceID) DisconnectGamepad();
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            if (event.gbutton.which == m_instanceID) {
                m_currentGamepadButtons[event.gbutton.button] = true;
            }
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            if (event.gbutton.which == m_instanceID) {
                m_currentGamepadButtons[event.gbutton.button] = false;
            }
            break;

        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            if (event.gaxis.which == m_instanceID) {
                float val = event.gaxis.value / (event.gaxis.value < 0 ? 32768.0f : 32767.0f);
                if (event.gaxis.axis < SDL_GAMEPAD_AXIS_COUNT) {
                    m_axes[event.gaxis.axis] = val;
                }
            }
            break;
    }
}

ButtonState InputManager::GetActionState(Action action) const {
    bool cur = false;
    bool prev = false;

    // Check keyboard mappings
    auto keyIt = m_keyboardMaps.find(action);
    if (keyIt != m_keyboardMaps.end()) {
        for (SDL_Keycode key : keyIt->second) {
            auto curKey = m_currentKeys.find(key);
            if (curKey != m_currentKeys.end() && curKey->second) {
                cur = true;
            }
            auto prevKey = m_previousKeys.find(key);
            if (prevKey != m_previousKeys.end() && prevKey->second) {
                prev = true;
            }
        }
    }

    // Check gamepad mappings
    auto padIt = m_gamepadMaps.find(action);
    if (padIt != m_gamepadMaps.end()) {
        for (SDL_GamepadButton btn : padIt->second) {
            if (m_currentGamepadButtons[btn]) cur = true;
            if (m_previousGamepadButtons[btn]) prev = true;
        }
    }

    // Evaluate state transition
    if (cur && !prev) return ButtonState::Pressed;
    if (cur && prev)  return ButtonState::Held;
    if (!cur && prev) return ButtonState::Released;
    return ButtonState::Up;
}

bool InputManager::IsPressed(Action action) const  { return GetActionState(action) == ButtonState::Pressed; }
bool InputManager::IsHeld(Action action) const     { return GetActionState(action) == ButtonState::Held; }
bool InputManager::IsReleased(Action action) const { return GetActionState(action) == ButtonState::Released; }

float InputManager::GetAxis(SDL_GamepadAxis axis, float deadzone) const {
    float val = m_axes[axis];
    if (std::abs(val) < deadzone) return 0.0f;
    float sign = (val > 0) ? 1.0f : -1.0f;
    return sign * (std::abs(val) - deadzone) / (1.0f - deadzone);
}

void InputManager::Rumble(float lowFreq, float highFreq, uint32_t durationMs) {
    if (m_gamepad) {
        uint16_t low = static_cast<uint16_t>(lowFreq * 0xFFFF);
        uint16_t high = static_cast<uint16_t>(highFreq * 0xFFFF);
        SDL_RumbleGamepad(m_gamepad, low, high, durationMs);
    }
}

void InputManager::ConnectGamepad(SDL_JoystickID joystickID) {
    m_gamepad = SDL_OpenGamepad(joystickID);
    if (m_gamepad) {
        m_instanceID = joystickID;
        std::cout << "Gamepad connected: " << SDL_GetGamepadName(m_gamepad) << std::endl;
    }
}

void InputManager::DisconnectGamepad() {
    if (m_gamepad) {
        std::cout << "Gamepad disconnected." << std::endl;
        SDL_CloseGamepad(m_gamepad);
        m_gamepad = nullptr;
        m_instanceID = 0;
    }
}

bool InputManager::IsGamepadConnected() const {
    return m_gamepad != nullptr;
}