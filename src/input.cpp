#include "input.h"
#include <algorithm>
#include <cstring>

InputManager::~InputManager() {
    for (auto gp : m_gamepads)
        SDL_GameControllerClose(gp);
}

void InputManager::on_joystick_added(int device_index) {
    if (!SDL_IsGameController(device_index)) return;
    SDL_GameController* gp = SDL_GameControllerOpen(device_index);
    if (!gp) return;
    m_gamepads.push_back(gp);
    if (!m_current)
        m_current = m_gamepads[m_index];
}

void InputManager::on_joystick_removed(SDL_JoystickID id) {
    m_gamepads.erase(
        std::remove_if(m_gamepads.begin(), m_gamepads.end(),
            [id](SDL_GameController* gp) {
                SDL_Joystick* j = SDL_GameControllerGetJoystick(gp);
                if (SDL_JoystickInstanceID(j) == id) {
                    SDL_GameControllerClose(gp);
                    return true;
                }
                return false;
            }),
        m_gamepads.end()
    );
    if (m_gamepads.empty()) {
        m_current = nullptr;
        m_index = 0;
    } else {
        m_index   = std::min(m_index, (int)m_gamepads.size() - 1);
        m_current = m_gamepads[m_index];
    }
}

void InputManager::cycle_gamepad() {
    if (m_gamepads.size() < 2) return;
    m_index   = (m_index + 1) % (int)m_gamepads.size();
    m_current = m_gamepads[m_index];
}

std::string InputManager::gamepad_name() const {
    if (!m_current) return "";
    const char* n = SDL_GameControllerName(m_current);
    return n ? n : "Unknown";
}

static float axis_norm(SDL_GameController* gp, SDL_GameControllerAxis axis) {
    return SDL_GameControllerGetAxis(gp, axis) / 32767.0f;
}

GamepadState InputManager::poll() const {
    GamepadState s;
    if (!m_current) return s;
    SDL_GameController* gp = m_current;

    s.leftX  = axis_norm(gp, SDL_CONTROLLER_AXIS_LEFTX);
    s.leftY  = axis_norm(gp, SDL_CONTROLLER_AXIS_LEFTY);
    s.rightX = axis_norm(gp, SDL_CONTROLLER_AXIS_RIGHTX);
    s.rightY = axis_norm(gp, SDL_CONTROLLER_AXIS_RIGHTY);

    // Triggers: -32768 at rest, 32767 at full : normalise to 0..1
    float lt = SDL_GameControllerGetAxis(gp, SDL_CONTROLLER_AXIS_TRIGGERLEFT)  / 32767.0f;
    float rt = SDL_GameControllerGetAxis(gp, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
    s.leftTrigger  = (lt + 1.0f) * 0.5f;
    s.rightTrigger = (rt + 1.0f) * 0.5f;

    auto btn = [&](SDL_GameControllerButton b) {
        return SDL_GameControllerGetButton(gp, b) == 1;
    };

    s.a      = btn(SDL_CONTROLLER_BUTTON_A);
    s.b      = btn(SDL_CONTROLLER_BUTTON_B);
    s.x      = btn(SDL_CONTROLLER_BUTTON_X);
    s.y      = btn(SDL_CONTROLLER_BUTTON_Y);
    s.select = btn(SDL_CONTROLLER_BUTTON_BACK);
    s.start  = btn(SDL_CONTROLLER_BUTTON_START);
    s.guide  = btn(SDL_CONTROLLER_BUTTON_GUIDE);
    s.lb     = btn(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    s.rb     = btn(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    s.l3     = btn(SDL_CONTROLLER_BUTTON_LEFTSTICK);
    s.r3     = btn(SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    s.dpUp   = btn(SDL_CONTROLLER_BUTTON_DPAD_UP);
    s.dpDown = btn(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    s.dpLeft = btn(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    s.dpRight= btn(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

    s.quit_requested  = s.select && s.start;
    s.cycle_requested = s.lb && s.rb;

    return s;
}