#pragma once
#include <SDL.h>
#include <string>
#include <vector>

struct GamepadState {
    float leftX  = 0, leftY  = 0;
    float rightX = 0, rightY = 0;
    float leftTrigger  = 0;
    float rightTrigger = 0;

    bool a = false, b = false, x = false, y = false;
    bool select = false, start = false, guide = false;
    bool lb = false, rb = false;
    bool l3 = false, r3 = false;
    bool dpUp = false, dpDown = false, dpLeft = false, dpRight = false;

    bool quit_requested  = false;  // select + start
    bool cycle_requested = false;  // lb + rb
};

class InputManager {
public:
    ~InputManager();

    void on_joystick_added(int device_index);
    void on_joystick_removed(SDL_JoystickID id);
    void cycle_gamepad();

    bool        has_gamepad()    const { return m_current != nullptr; }
    std::string gamepad_name()   const;
    int         gamepad_count()  const { return (int)m_gamepads.size(); }

    GamepadState poll() const;

private:
    std::vector<SDL_GameController*> m_gamepads;
    SDL_GameController* m_current = nullptr;
    int m_index = 0;
};