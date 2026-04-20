#include "renderer.h"
#include <cmath>
#include <cstdio>
#include <SDL2_gfxPrimitives.h>

static constexpr float PI = 3.14159265f;

void Renderer::draw_circle_outline(int cx, int cy, int r) {
    uint8_t rr, gg, bb, aa;
    SDL_GetRenderDrawColor(m_ren, &rr, &gg, &bb, &aa);
    circleRGBA(m_ren, cx, cy, r, rr, gg, bb, aa);
}

// Uusing GFXPrimitives for more optimised rendering
void Renderer::draw_filled_circle(int cx, int cy, int r) {
    uint8_t rr, gg, bb, aa;
    SDL_GetRenderDrawColor(m_ren, &rr, &gg, &bb, &aa);
    filledCircleRGBA(m_ren, cx, cy, r, rr, gg, bb, aa);
}

bool Renderer::init(SDL_Renderer* ren, const Config& cfg) {
    m_ren = ren;
    m_cfg = cfg;
    if (TTF_Init() != 0) return false;
    m_font = TTF_OpenFont("assets/font.ttf", 14);
    if (!m_font) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void Renderer::shutdown() {
    clear_text_cache();
    if (m_font) { TTF_CloseFont(m_font); m_font = nullptr; }
    TTF_Quit();
}

void Renderer::begin_frame() {
    SDL_SetRenderDrawColor(m_ren, 0, 0, 0, 255);
    SDL_RenderClear(m_ren);
}

void Renderer::end_frame() {
    SDL_RenderPresent(m_ren);
}

// helper method for text rendering with caching to improve performance
void Renderer::print(const std::string& text, int x, int y,
                     uint8_t r, uint8_t g, uint8_t b) {
    if (!m_font) return;

    // Cache key includes colour so its reusable for different coloured texts
    char key_buf[256];
    snprintf(key_buf, sizeof(key_buf), "%s_%d_%d_%d", text.c_str(), r, g, b);
    std::string key(key_buf);

    auto it = m_text_cache.find(key);
    if (it == m_text_cache.end()) {
        // if not cached, render the text surface and create a texture (once)
        SDL_Color col = {r, g, b, 255};
        SDL_Surface* surf = TTF_RenderText_LCD(m_font, text.c_str(),
                                               col, {0, 0, 0, 255});
        if (!surf) return;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(m_ren, surf);
        SDL_FreeSurface(surf);
        if (!tex) return;

        CachedText ct;
        ct.texture = tex;
        SDL_QueryTexture(tex, nullptr, nullptr, &ct.w, &ct.h);
        m_text_cache[key] = ct;
        it = m_text_cache.find(key);
    }

    SDL_Rect dst = {x, y, it->second.w, it->second.h};
    SDL_RenderCopy(m_ren, it->second.texture, nullptr, &dst);
}

void Renderer::clear_text_cache() {
    for (auto& [key, ct] : m_text_cache)
        SDL_DestroyTexture(ct.texture);
    m_text_cache.clear();
}

void Renderer::update_fps(uint32_t now_ms) {
    m_fps_count++;
    if (now_ms - m_fps_last_time >= 1000) {
        m_fps           = m_fps_count;
        m_fps_count     = 0;
        m_fps_last_time = now_ms;
    }
}

void Renderer::draw_stick(int cx, int cy, const StickTrail& trail, bool pressed,
                           uint8_t tr, uint8_t tg, uint8_t tb, uint32_t now_ms)
{
    int range = (int)m_cfg.stick_range;

    // outer ring
    SDL_SetRenderDrawColor(m_ren, 50, 50, 50, 255);
    draw_circle_outline(cx, cy, range);

    // trail points (gfxPrimitives)
    for (auto& pt : trail.points()) {
        float age = float(now_ms - pt.timestamp_ms);
        uint8_t alpha = 255;
        if (age > m_cfg.trail_duration_ms) {
            float t = (age - m_cfg.trail_duration_ms) / m_cfg.fade_duration_ms;
            alpha = (uint8_t)(255.0f * (1.0f - t));
        }

        int px = cx + (int)(pt.x * range);
        int py = cy + (int)(pt.y * range);

        filledCircleRGBA(m_ren, px, py, 5, tr, tg, tb, alpha);
    }

    if (trail.points().empty()) return;

    auto& cur = trail.points().back();
    int sx = cx + (int)(cur.x * range);
    int sy = cy + (int)(cur.y * range);

    // line from center to stick pos
    SDL_SetRenderDrawColor(m_ren, 180, 180, 180, 128);
    SDL_RenderDrawLine(m_ren, cx, cy, sx, sy);

    // center dot based on pressed state
    if (pressed) {
        SDL_SetRenderDrawColor(m_ren, 255, 255, 255, 255);
        draw_filled_circle(sx, sy, 15);
    } else {
        SDL_SetRenderDrawColor(m_ren, 50, 50, 50, 255);
        draw_filled_circle(sx, sy, 7);
    }
}

void Renderer::draw_buttons(const GamepadState& s) {
    auto circ = [&](bool active, int cx, int cy,
                    uint8_t ar, uint8_t ag, uint8_t ab) {
        SDL_SetRenderDrawColor(m_ren,
            active ? ar : 50,
            active ? ag : 50,
            active ? ab : 50, 255);
        active ? draw_filled_circle(cx, cy, BTN_R)
               : draw_circle_outline(cx, cy, BTN_R);
    };

    circ(s.a,      BTN_X + BTN_OFF, BTN_Y,          220,  50,  50); // A right  red
    circ(s.b,      BTN_X,           BTN_Y + BTN_OFF, 220, 220,  50); // B bottom yellow
    circ(s.x,      BTN_X,           BTN_Y - BTN_OFF,  50,  50, 220); // X top    blue
    circ(s.y,      BTN_X - BTN_OFF, BTN_Y,            50, 220,  50); // Y left   green
    circ(s.select, 240, 180,                          180, 180, 180);
    circ(s.start,  400, 180,                          180, 180, 180);
    circ(s.guide,  320, 120,                          180, 180, 180);

    auto rect = [&](bool active, int x, int y, int w, int h) {
        SDL_SetRenderDrawColor(m_ren,
            active ? 160 : 50, active ? 160 : 50, active ? 160 : 50, 255);
        SDL_Rect r = {x, y, w, h};
        active ? SDL_RenderFillRect(m_ren, &r) : SDL_RenderDrawRect(m_ren, &r);
    };

    rect(s.lb, 40,  60, 30, 30);
    rect(s.rb, 570, 60, 30, 30);
}

void Renderer::draw_triggers(const GamepadState& s) {
    auto trigger = [&](int x, float value) {
        SDL_Rect outline = {x, 20, 60, 30};
        SDL_SetRenderDrawColor(m_ren, 50, 50, 50, 255);
        SDL_RenderDrawRect(m_ren, &outline);
        if (value > 0.01f) {
            SDL_Rect fill = {x, 20, (int)(60.0f * value), 30};
            SDL_SetRenderDrawColor(m_ren, 255, 255, 255, 255);
            SDL_RenderFillRect(m_ren, &fill);
        }
    };
    trigger(120, s.leftTrigger);
    trigger(460, s.rightTrigger);
}

void Renderer::draw_dpad(const GamepadState& s) {
    // Drawn cross, TODO replace with PNG sprites later
    const int cx = DPAD_X, cy = DPAD_CY;
    const int arm = 25, thick = 14;

    auto arm_rect = [&](bool active, int rx, int ry, int rw, int rh) {
        SDL_SetRenderDrawColor(m_ren,
            active ? 200 : 60, active ? 200 : 60, active ? 200 : 60, 255);
        SDL_Rect r = {rx, ry, rw, rh};
        active ? SDL_RenderFillRect(m_ren, &r) : SDL_RenderDrawRect(m_ren, &r);
    };

    arm_rect(s.dpUp,    cx - thick/2, cy - arm - thick/2, thick, arm);
    arm_rect(s.dpDown,  cx - thick/2, cy + thick/2,        thick, arm);
    arm_rect(s.dpLeft,  cx - arm - thick/2, cy - thick/2,  arm,   thick);
    arm_rect(s.dpRight, cx + thick/2,       cy - thick/2,  arm,   thick);

    SDL_SetRenderDrawColor(m_ren, 60, 60, 60, 255);
    SDL_Rect center = {cx - thick/2, cy - thick/2, thick, thick};
    SDL_RenderFillRect(m_ren, &center);
}

void Renderer::draw(const GamepadState& state,
                    const StickTrail& lt, const StickTrail& rt,
                    int gamepad_count, const std::string& gamepad_name,
                    uint32_t now_ms)
{
    update_fps(now_ms);
    draw_stick(LS_CX, LS_CY, lt, state.l3,   0, 255,   0, now_ms);
    draw_stick(RS_CX, RS_CY, rt, state.r3, 255,   0,   0, now_ms);
    draw_buttons(state);
    draw_triggers(state);
    draw_dpad(state);

    // Raw axis values, rate limited to avoid excessive text rendering and cache churn
    if (now_ms - m_axis_update_time > 100) {
        m_axis_update_time = now_ms;
        if (!lt.points().empty()) {
            auto& p = lt.points().back();
            char buf[32];
            snprintf(buf, sizeof(buf), "(%.2f, %.2f)", p.x, p.y);
            m_left_axis_str = buf;
        }
        if (!rt.points().empty()) {
            auto& p = rt.points().back();
            char buf[32];
            snprintf(buf, sizeof(buf), "(%.2f, %.2f)", p.x, p.y);
            m_right_axis_str = buf;
        }
        // Clear only the axis cache entries so stale values dont accumulate
        // (static strings like "Connected: ..." stay cached forever)
        for (auto it = m_text_cache.begin(); it != m_text_cache.end(); ) {
            if (it->first.find("(") != std::string::npos) {
                SDL_DestroyTexture(it->second.texture);
                it = m_text_cache.erase(it);
            } else {
                ++it;
            }
        }
    }
    if (!m_left_axis_str.empty())  print(m_left_axis_str, 110, 410);
    if (!m_right_axis_str.empty()) print(m_right_axis_str, 430, 410);

    print("Press Select + Start to quit", 190, 50);

    if (!gamepad_name.empty()) {
        print("Connected: " + gamepad_name, 20, 440);
        if (gamepad_count > 1)
            print("LB + RB to cycle gamepads", 20, 460);
    } else {
        print("No gamepad detected", 240, 240);
    }

    print("github/jxded | youtube/@zakstechs", 390, 460, 100, 100, 100);

    char fps_buf[16];
    snprintf(fps_buf, sizeof(fps_buf), "FPS: %d", m_fps);
    print(fps_buf, m_cfg.screen_w - 70, 8, 80, 80, 80);
}