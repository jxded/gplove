#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "input.h"
#include "trail.h"
#include <unordered_map>


struct Config {
    float trail_duration_ms = 750.0f;
    float fade_duration_ms  =  300.0f;
    float stick_range       =   80.0f;  // px, center to edge
    int   screen_w          =  640;
    int   screen_h          =  480;
};

struct CachedText {
    SDL_Texture* texture = nullptr;
    int w = 0, h = 0;
};

class Renderer {
public:
    bool init(SDL_Renderer* ren, const Config& cfg);
    void shutdown();

    void begin_frame();
    void draw(const GamepadState& state,
              const StickTrail& left_trail,
              const StickTrail& right_trail,
              int gamepad_count,
              const std::string& gamepad_name,
              uint32_t now_ms);
    void end_frame();

private:
    void draw_circle_outline(int cx, int cy, int r);
    void draw_filled_circle(int cx, int cy, int r);
    void draw_stick(int cx, int cy, const StickTrail& trail, bool pressed,
                    uint8_t tr, uint8_t tg, uint8_t tb, uint32_t now_ms);
    void draw_buttons(const GamepadState& s);
    void draw_triggers(const GamepadState& s);
    void draw_dpad(const GamepadState& s);
    void print(const std::string& text, int x, int y,
               uint8_t r = 255, uint8_t g = 255, uint8_t b = 255);
    
    SDL_Renderer* m_ren  = nullptr;
    TTF_Font*     m_font = nullptr;
    Config        m_cfg;
    
    // FPS display variables
    uint32_t m_fps_last_time = 0;
    int      m_fps_count     = 0;
    int      m_fps           = 0;
    void update_fps(uint32_t now_ms);
    
    // Helper methods to cache rendered text textures for performance
    std::unordered_map<std::string, CachedText> m_text_cache;
    void clear_text_cache();

    // forced axis string update rate
    uint32_t m_axis_update_time = 0;
    std::string m_left_axis_str;
    std::string m_right_axis_str;

    // point buffers preallocated
    std::vector<SDL_Point> m_point_buf;
    std::vector<SDL_Point> m_circle_buf;


    // Button layout center coordinate constants
    static constexpr int LS_CX  = 160, LS_CY  = 320;
    static constexpr int RS_CX  = 480, RS_CY  = 320;
    static constexpr int BTN_X  = 520, BTN_Y  = 135, BTN_OFF = 35, BTN_R = 15;
    static constexpr int DPAD_X =  160, DPAD_CY = 130;
};