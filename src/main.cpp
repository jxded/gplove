#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdio>
#include "input.h"
#include "renderer.h"
#include "trail.h"

int main(int /*argc*/, char* /*argv*/[]) {
    // best, at least on metal, to not do forced fps caps as it conflicts with metal's internal
    // frame pacing (vsync..) just rely on SDL_RENDERER_PRESENTVSYNC for now
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // we're loading the SDL community gamecontroller mappings. 
    // mainly doing this to match portmaster port conventions, 
    // but it also helps during prototyping on desktop.
    SDL_GameControllerAddMappingsFromFile("assets/gamecontrollerdb.txt");


    // SDL_WINDOW_SHOWN for desktop and SDL_WINDOW_FULLSCREEN for device
    // MuOS at least mentions there is no DWM, so we try and access KMS directly
    // Swap when building for device target!
    SDL_Window* window = SDL_CreateWindow(
        "gplove",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
        // SDL_WINDOW_FULLSCREEN 
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0"); // prevents fuzzy rendering on retina

    SDL_Renderer* ren = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    // set logical size to 640x480, let SDL handle scaling
    // high dpi screens like retina render with 2x pixel density
    int draw_w, draw_h;
    SDL_GetRendererOutputSize(ren, &draw_w, &draw_h);
    SDL_RenderSetLogicalSize(ren, 640, 480);

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    Config   cfg;
    Renderer renderer;
    if (!renderer.init(ren, cfg)) {
        fprintf(stderr, "Renderer init failed, is assets/font.ttf present?\n");
        return 1;
    }

    InputManager input;
    StickTrail   left_trail, right_trail;
    bool         running = true;
    SDL_Event    ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_CONTROLLERDEVICEADDED:
                    input.on_joystick_added(ev.cdevice.which);
                    break;
                case SDL_CONTROLLERDEVICEREMOVED:
                    input.on_joystick_removed(ev.cdevice.which);
                    break;
                case SDL_KEYDOWN:
                    if (ev.key.keysym.sym == SDLK_ESCAPE) running = false;
                    break;
            }
        }

        uint32_t     now   = SDL_GetTicks();
        GamepadState state = input.poll();

        if (state.quit_requested)  running = false;
        if (state.cycle_requested) input.cycle_gamepad();

        left_trail.push(state.leftX, state.leftY, now);
        right_trail.push(state.rightX, state.rightY, now);
        left_trail.prune(now, cfg.trail_duration_ms, cfg.fade_duration_ms);
        right_trail.prune(now, cfg.trail_duration_ms, cfg.fade_duration_ms);

        renderer.begin_frame();
        renderer.draw(state, left_trail, right_trail,
                      input.gamepad_count(), input.gamepad_name(), now);
        renderer.end_frame();
    }

    renderer.shutdown();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}