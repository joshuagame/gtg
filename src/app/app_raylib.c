#include "app/app.h"
#include "app/app_internal.h"

#include "raylib.h"

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

enum
{
    RAYLIB_DIAGNOSTICS_CAPACITY = 256,
    RAYLIB_PROMPT_CAPACITY = CONSOLE_INPUT_CAPACITY + 3,
};

typedef struct
{
    Font font;
    char diagnostics[RAYLIB_DIAGNOSTICS_CAPACITY];
    double next_diagnostics_update;
    uint64_t last_tick_count;
    AppState last_state;
    bool last_paused;
    bool diagnostics_ready;
    bool font_owned;
} RaylibRuntime;

static void raylib_configure_window(void *context, bool resizable, bool fullscreen)
{
    unsigned int flags = 0U;
    (void)context;
    if (resizable)
        flags |= FLAG_WINDOW_RESIZABLE;
    if (fullscreen)
        flags |= FLAG_FULLSCREEN_MODE;
    if (flags != 0U)
        SetConfigFlags(flags);
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
    AppInput input = {0};
    int codepoint = GetCharPressed();
    (void)context;

    while (codepoint > 0)
    {
        if (codepoint >= 32 && codepoint <= 126 &&
            input.console.text_length + 1U < sizeof(input.console.text))
        {
            input.console.text[input.console.text_length] = (char)codepoint;
            input.console.text_length += 1U;
            input.console.text[input.console.text_length] = '\0';
        }
        codepoint = GetCharPressed();
    }

    input.console.backspace = IsKeyPressed(KEY_BACKSPACE);
    input.console.submit = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    input.console.history_previous = IsKeyPressed(KEY_UP);
    input.console.history_next = IsKeyPressed(KEY_DOWN);
    input.console.toggle_typing = IsKeyPressed(KEY_F4);
    input.start_game = IsKeyPressed(KEY_F1);
    input.toggle_pause = IsKeyPressed(KEY_F2);
    input.step_once = IsKeyPressed(KEY_F3);
    input.exit_requested = IsKeyPressed(KEY_F10);
    return input;
}

static bool raylib_load_resources(void *context, Resources *resources)
{
    static const char font_path[] = "assets/fonts/ShareTechMono-Regular.ttf";
    RaylibRuntime *runtime = context;

    if (!resources_load(resources))
        return false;

    runtime->font = GetFontDefault();
    runtime->font_owned = false;
    if (FileExists(font_path))
    {
        const Font loaded_font = LoadFontEx(font_path, 24, NULL, 0);
        if (IsFontValid(loaded_font))
        {
            runtime->font = loaded_font;
            runtime->font_owned = true;
        }
    }
    return true;
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

static void raylib_draw_text(Font font, const char *text, float x, float y, float size, Color color)
{
    DrawTextEx(font, text, (Vector2){x, y}, size, 1.0F, color);
}

static void raylib_draw_console(const RaylibRuntime *runtime, const AppView *view, int screen_width,
                                int screen_height)
{
    static const Color phosphor_green = {94, 255, 154, 255};
    static const Color dim_green = {45, 130, 80, 255};
    static const float text_size = 20.0F;
    static const float line_height = 25.0F;
    static const int margin = 32;
    static const int output_top = 106;
    static const int footer_height = 42;
    static const int prompt_height = 38;
    char prompt[RAYLIB_PROMPT_CAPACITY] = {0};
    const int panel_width = screen_width > margin * 2 ? screen_width - margin * 2 : 0;
    const int prompt_y = screen_height - footer_height - prompt_height;
    const int output_height = prompt_y > output_top ? prompt_y - output_top : 0;
    const size_t visible_rows = (size_t)((float)output_height / line_height);
    const size_t output_count = console_output_count(view->console);
    const size_t first_line = output_count > visible_rows ? output_count - visible_rows : 0U;

    BeginScissorMode(margin, output_top, panel_width, output_height);
    for (size_t index = first_line; index < output_count; index += 1U)
    {
        const ConsoleLine *line = console_output_line(view->console, index);
        char visible_text[CONSOLE_LINE_CAPACITY] = {0};
        const size_t row = index - first_line;
        if (line == NULL)
            continue;

        for (size_t character = 0U; character < line->visible_length; character += 1U)
        {
            visible_text[character] = line->text[character];
        }
        visible_text[line->visible_length] = '\0';
        raylib_draw_text(runtime->font, visible_text, (float)margin,
                         (float)output_top + (float)row * line_height, text_size, phosphor_green);
    }
    EndScissorMode();

    (void)snprintf(prompt, sizeof(prompt), "> %s", console_input_text(view->console));
    BeginScissorMode(margin, prompt_y, panel_width, prompt_height);
    raylib_draw_text(runtime->font, prompt, (float)margin, (float)prompt_y, text_size,
                     phosphor_green);
    if (fmod(GetTime(), 1.0) < 0.55)
    {
        const Vector2 prompt_size = MeasureTextEx(runtime->font, prompt, text_size, 1.0F);
        DrawRectangle((int)((float)margin + prompt_size.x + 2.0F), prompt_y + 3, 11, 20,
                      phosphor_green);
    }
    EndScissorMode();
    DrawLine(margin, prompt_y - 8, screen_width - margin, prompt_y - 8, dim_green);
}

static void raylib_update_diagnostics(RaylibRuntime *runtime, const AppView *view,
                                      double current_time)
{
    static const double diagnostics_interval_seconds = 0.25;
    const bool state_changed = !runtime->diagnostics_ready || runtime->last_state != view->state ||
                               runtime->last_paused != view->paused;
    const bool paused_tick_changed = view->paused && runtime->last_tick_count != view->tick_count;
    const bool refresh_due = current_time >= runtime->next_diagnostics_update;

    if (!state_changed && !paused_tick_changed && !refresh_due)
        return;

    (void)snprintf(runtime->diagnostics, sizeof(runtime->diagnostics),
                   "STATE:%-4s | FPS:%3d | TICK/S:%3d | TICKS:%010" PRIu64
                   " | FRAME TICKS:%02u | ALPHA:%4.2f | %-7s | TYPE:%s",
                   app_state_string(view->state), view->frames_per_second, view->ticks_per_second,
                   view->tick_count, view->ticks_last_frame, (double)view->interpolation_alpha,
                   view->paused ? "PAUSED" : "RUNNING",
                   console_typing_enabled(view->console) ? "ON " : "OFF");
    runtime->next_diagnostics_update = current_time + diagnostics_interval_seconds;
    runtime->last_tick_count = view->tick_count;
    runtime->last_state = view->state;
    runtime->last_paused = view->paused;
    runtime->diagnostics_ready = true;
}

static void raylib_draw_overlay(void *context, const AppView *view)
{
    static const Color phosphor_green = {94, 255, 154, 255};
    static const Color dim_green = {45, 130, 80, 255};
    RaylibRuntime *runtime = context;
    const int screen_width = GetScreenWidth();
    const int screen_height = GetScreenHeight();

    raylib_draw_text(runtime->font, "WOPR // GLOBAL THERMONUCLEAR WAR", 32.0F, 24.0F, 28.0F,
                     phosphor_green);
    raylib_draw_text(runtime->font,
                     "F1 GAME  F2 PAUSE  F3 STEP  F4 TYPEWRITER  F10 QUIT  UP/DOWN HISTORY", 32.0F,
                     67.0F, 16.0F, dim_green);
    raylib_draw_console(runtime, view, screen_width, screen_height);
    raylib_update_diagnostics(runtime, view, GetTime());

    BeginScissorMode(32, screen_height - 40, screen_width > 64 ? screen_width - 64 : 0, 24);
    raylib_draw_text(runtime->font, runtime->diagnostics, 32.0F, (float)(screen_height - 38), 16.0F,
                     dim_green);
    EndScissorMode();
}

static void raylib_end_drawing(void *context)
{
    (void)context;
    EndDrawing();
}

static void raylib_unload_resources(void *context, Resources *resources)
{
    RaylibRuntime *runtime = context;
    if (runtime->font_owned)
        UnloadFont(runtime->font);
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
