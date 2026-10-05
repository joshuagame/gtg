#include "app/app.h"
#include "app/app_internal.h"

#include "raylib.h"

#include <stdbool.h>
#include <stddef.h>

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
    const AppBackend backend = {
        .context = NULL,
        .configure_window = raylib_configure_window,
        .init_window = raylib_init_window,
        .is_window_ready = raylib_is_window_ready,
        .set_target_fps = raylib_set_target_fps,
        .window_should_close = raylib_window_should_close,
        .load_resources = raylib_load_resources,
        .begin_drawing = raylib_begin_drawing,
        .clear_background = raylib_clear_background,
        .end_drawing = raylib_end_drawing,
        .unload_resources = raylib_unload_resources,
        .close_window = raylib_close_window,
    };

    return app_run_with_backend(config, &backend);
}
