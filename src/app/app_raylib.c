#include "app/app.h"
#include "app/app_internal.h"

#include "raylib.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

enum
{
    RAYLIB_DIAGNOSTICS_CAPACITY = 256,
};

typedef struct
{
    char diagnostics[RAYLIB_DIAGNOSTICS_CAPACITY];
    double next_diagnostics_update;
    uint64_t last_tick_count;
    AppState last_state;
    bool last_paused;
    bool diagnostics_ready;
} RaylibRuntime;

static void raylib_configure_window(void *context, bool resizable, bool fullscreen)
{
    unsigned int flags = 0U;
    (void)context;

    if (resizable)
    {
        flags |= FLAG_WINDOW_RESIZABLE;
    }

    if (fullscreen)
    {
        flags |= FLAG_FULLSCREEN_MODE;
    }

    if (flags != 0U)
    {
        SetConfigFlags(flags);
    }
}

static void raylib_init_window(void *context, int width, int height, const char *title)
{
    (void)context;
    InitWindow(width, height, title);
}

static bool raylib_is_window_ready(void *context)
{
    (void)context;
    return IsWindowReady();
}

static void raylib_set_target_fps(void *context, int target_fps)
{
    (void)context;
    SetTargetFPS(target_fps);
}

static bool raylib_window_should_close(void *context)
{
    (void)context;
    return WindowShouldClose();
}

static double raylib_get_frame_time(void *context)
{
    (void)context;
    return (double)GetFrameTime();
}

static int raylib_get_fps(void *context)
{
    (void)context;
    return GetFPS();
}

static AppInput raylib_read_input(void *context)
{
    (void)context;
    return (AppInput){
        .toggle_pause = IsKeyPressed(KEY_P),
        .step_once = IsKeyPressed(KEY_N),
        .start_game = IsKeyPressed(KEY_ENTER),
        .exit_requested = IsKeyPressed(KEY_Q),
    };
}

static bool raylib_load_resources(void *context, Resources *resources)
{
    (void)context;
    return resources_load(resources);
}

static void raylib_begin_drawing(void *context)
{
    (void)context;
    BeginDrawing();
}

static void raylib_clear_background(void *context)
{
    static const Color wopr_background = {3, 12, 8, 255};
    (void)context;
    ClearBackground(wopr_background);
}

static void raylib_draw_overlay(void *context, const AppView *view)
{
    static const Color phosphor_green = {94, 255, 154, 255};
    static const Color dim_green = {45, 130, 80, 255};
    static const double diagnostics_interval_seconds = 0.25;
    RaylibRuntime *runtime = context;
    const char *state_hint = "INITIALIZING WOPR...";
    const double current_time = GetTime();
    const bool state_changed = !runtime->diagnostics_ready || runtime->last_state != view->state ||
                               runtime->last_paused != view->paused;
    const bool paused_tick_changed = view->paused && runtime->last_tick_count != view->tick_count;
    const bool refresh_due = current_time >= runtime->next_diagnostics_update;

    if (view->state == APP_STATE_MENU)
    {
        state_hint = "PRESS ENTER TO START";
    }
    else if (view->state == APP_STATE_GAME)
    {
        state_hint = "SIMULATION READY";
    }

    DrawText("WOPR // GLOBAL THERMONUCLEAR WAR", 32, 28, 28, phosphor_green);
    DrawText(state_hint, 32, 78, 22, phosphor_green);
    DrawText("P: PAUSE   N: SINGLE TICK   Q: QUIT", 32, 116, 18, dim_green);

    if (state_changed || paused_tick_changed || refresh_due)
    {
        (void)snprintf(runtime->diagnostics, sizeof(runtime->diagnostics),
                       "STATE:%-4s | FPS:%3d | TICK/S:%3d | TICKS:%010" PRIu64
                       " | FRAME TICKS:%02u | ALPHA:%4.2f | %-7s",
                       app_state_string(view->state), view->frames_per_second,
                       view->ticks_per_second, view->tick_count, view->ticks_last_frame,
                       (double)view->interpolation_alpha, view->paused ? "PAUSED" : "RUNNING");
        runtime->next_diagnostics_update = current_time + diagnostics_interval_seconds;
        runtime->last_tick_count = view->tick_count;
        runtime->last_state = view->state;
        runtime->last_paused = view->paused;
        runtime->diagnostics_ready = true;
    }

    DrawText(runtime->diagnostics, 32, GetScreenHeight() - 40, 16, dim_green);
}

static void raylib_end_drawing(void *context)
{
    (void)context;
    EndDrawing();
}

static void raylib_unload_resources(void *context, Resources *resources)
{
    (void)context;
    resources_unload(resources);
}

static void raylib_close_window(void *context)
{
    (void)context;
    CloseWindow();
}

AppResult app_run(const AppConfig *config)
{
    RaylibRuntime runtime = {0};
    const AppBackend backend = {
        .context = &runtime,
        .configure_window = raylib_configure_window,
        .init_window = raylib_init_window,
        .is_window_ready = raylib_is_window_ready,
        .set_target_fps = raylib_set_target_fps,
        .window_should_close = raylib_window_should_close,
        .get_frame_time = raylib_get_frame_time,
        .get_fps = raylib_get_fps,
        .read_input = raylib_read_input,
        .load_resources = raylib_load_resources,
        .begin_drawing = raylib_begin_drawing,
        .clear_background = raylib_clear_background,
        .draw_overlay = raylib_draw_overlay,
        .end_drawing = raylib_end_drawing,
        .unload_resources = raylib_unload_resources,
        .close_window = raylib_close_window,
    };

    return app_run_with_backend(config, &backend);
}
