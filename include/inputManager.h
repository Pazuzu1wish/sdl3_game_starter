// inputManager.h
#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>

enum class Action {
    Confirm,
    Cancel,
    Pause,
    Up,
    Down,
    Left,
    Right
};

enum class ButtonState {
    Up,
    Pressed,
    Held,
    Released
};

class InputManager {
public:
    InputManager();
    ~InputManager();

    // Prevent copies of the manager
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void BeginFrame();
    void ProcessEvent(const SDL_Event& event);

    // Query Actions (Combines Keyboard + Gamepad)
    bool IsPressed(Action action) const;
    bool IsHeld(Action action) const;
    bool IsReleased(Action action) const;

    // Analog stick query (Gamepad specific)
    float GetAxis(SDL_GamepadAxis axis, float deadzone = 0.15f) const;

    // Rumble support
    void Rumble(float lowFreq, float highFreq, uint32_t durationMs);

    bool IsGamepadConnected() const;

private:
    void ConnectGamepad(SDL_JoystickID joystickID);
    void DisconnectGamepad();
    void SetupDefaultMappings();
    ButtonState GetActionState(Action action) const;

    // Action mappings (One action can map to multiple keys/buttons)
    std::unordered_map<Action, std::vector<SDL_Keycode>> m_keyboardMaps;
    std::unordered_map<Action, std::vector<SDL_GamepadButton>> m_gamepadMaps;

    // Keyboard states
    std::unordered_map<SDL_Keycode, bool> m_currentKeys;
    std::unordered_map<SDL_Keycode, bool> m_previousKeys;

    // Gamepad states
    SDL_Gamepad* m_gamepad = nullptr;
    SDL_JoystickID m_instanceID = 0;
    bool m_currentGamepadButtons[SDL_GAMEPAD_BUTTON_COUNT] = { false };
    bool m_previousGamepadButtons[SDL_GAMEPAD_BUTTON_COUNT] = { false };
    float m_axes[SDL_GAMEPAD_AXIS_COUNT] = { 0.0f };
};