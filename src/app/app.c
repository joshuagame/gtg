#include "app/app_internal.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    AppConfig config;
    Resources resources;
    AppState state;
    bool window_initialized;
    bool resources_loaded;
    bool paused;
    bool step_requested;
    double accumulator_seconds;
    double tick_seconds;
    uint64_t frames_drawn;
    uint64_t tick_count;
    unsigned int ticks_last_frame;
} App;

static bool app_backend_is_valid(const AppBackend *backend)
{
    return backend != NULL && backend->configure_window != NULL && backend->init_window != NULL &&
           backend->is_window_ready != NULL && backend->set_target_fps != NULL &&
           backend->window_should_close != NULL && backend->get_frame_time != NULL &&
           backend->get_fps != NULL && backend->read_input != NULL &&
           backend->load_resources != NULL && backend->begin_drawing != NULL &&
           backend->clear_background != NULL && backend->draw_overlay != NULL &&
           backend->end_drawing != NULL && backend->unload_resources != NULL &&
           backend->close_window != NULL;
}

static bool app_frame_limit_reached(const App *app)
{
    return app->config.frame_limit != 0U && app->frames_drawn >= app->config.frame_limit;
}

static double app_clamp_frame_time(const App *app, double frame_seconds)
{
    if (!isfinite(frame_seconds) || frame_seconds < 0.0)
    {
        return 0.0;
    }

    if (frame_seconds > app->config.max_frame_seconds)
    {
        return app->config.max_frame_seconds;
    }

    return frame_seconds;
}

static AppInput app_read_input(const AppBackend *backend)
{
    return backend->read_input(backend->context);
}

static void app_handle_input(App *app, AppInput input)
{
    if (input.exit_requested)
    {
        app->state = APP_STATE_EXIT;
        return;
    }

    if (input.toggle_pause)
    {
        app->paused = !app->paused;
    }

    if (input.step_once && app->paused)
    {
        app->step_requested = true;
    }

    if (input.start_game && app->state == APP_STATE_MENU)
    {
        app->state = APP_STATE_GAME;
    }
}

static void app_fixed_update(App *app)
{
    app->tick_count += 1U;

    switch (app->state)
    {
    case APP_STATE_BOOT:
        app->state = APP_STATE_MENU;
        break;
    case APP_STATE_MENU:
    case APP_STATE_GAME:
    case APP_STATE_EXIT:
        break;
    }
}

static void app_run_fixed_updates(App *app, double frame_seconds)
{
    static const double time_epsilon = 1.0e-12;
    app->ticks_last_frame = 0U;

    if (app->paused)
    {
        if (app->step_requested)
        {
            app_fixed_update(app);
            app->ticks_last_frame = 1U;
            app->step_requested = false;
        }
        return;
    }

    app->step_requested = false;
    app->accumulator_seconds += frame_seconds;

    while (app->accumulator_seconds + time_epsilon >= app->tick_seconds)
    {
        app_fixed_update(app);
        app->accumulator_seconds -= app->tick_seconds;
        app->ticks_last_frame += 1U;
    }

    if (app->accumulator_seconds < 0.0)
    {
        app->accumulator_seconds = 0.0;
    }
}

static float app_interpolation_alpha(const App *app)
{
    double alpha = app->accumulator_seconds / app->tick_seconds;

    if (alpha < 0.0)
    {
        alpha = 0.0;
    }
    else if (alpha > 1.0)
    {
        alpha = 1.0;
    }

    return (float)alpha;
}

static void app_draw(const App *app, const AppBackend *backend)
{
    const AppView view = {
        .state = app->state,
        .paused = app->paused,
        .tick_count = app->tick_count,
        .ticks_last_frame = app->ticks_last_frame,
        .ticks_per_second = app->paused ? 0 : app->config.simulation_hz,
        .frames_per_second = backend->get_fps(backend->context),
        .interpolation_alpha = app_interpolation_alpha(app),
    };

    backend->begin_drawing(backend->context);
    backend->clear_background(backend->context);
    backend->draw_overlay(backend->context, &view);
    backend->end_drawing(backend->context);
}

AppResult app_run_with_backend(const AppConfig *config, const AppBackend *backend)
{
    AppResult result = APP_RESULT_OK;
    App app = {0};

    if (!app_config_is_valid(config))
    {
        return APP_RESULT_INVALID_CONFIG;
    }

    if (!app_backend_is_valid(backend))
    {
        return APP_RESULT_INVALID_BACKEND;
    }

    app.config = *config;
    app.state = APP_STATE_BOOT;
    app.tick_seconds = 1.0 / (double)app.config.simulation_hz;

    backend->configure_window(backend->context, app.config.resizable, app.config.fullscreen);
    backend->init_window(backend->context, app.config.window_width, app.config.window_height,
                         app.config.window_title);
    app.window_initialized = true;

    if (!backend->is_window_ready(backend->context))
    {
        result = APP_RESULT_WINDOW_INIT_FAILED;
        goto cleanup;
    }

    backend->set_target_fps(backend->context, app.config.target_fps);

    if (!backend->load_resources(backend->context, &app.resources))
    {
        result = APP_RESULT_RESOURCE_LOAD_FAILED;
        goto cleanup;
    }
    app.resources_loaded = true;

    while (!app_frame_limit_reached(&app) && !backend->window_should_close(backend->context))
    {
        const double frame_seconds =
            app_clamp_frame_time(&app, backend->get_frame_time(backend->context));
        const AppInput input = app_read_input(backend);

        app_handle_input(&app, input);
        if (app.state == APP_STATE_EXIT)
        {
            break;
        }

        app_run_fixed_updates(&app, frame_seconds);
        app_draw(&app, backend);
        app.frames_drawn += 1U;
    }

cleanup:
    if (app.resources_loaded)
    {
        backend->unload_resources(backend->context, &app.resources);
    }

    if (app.window_initialized)
    {
        backend->close_window(backend->context);
    }

    return result;
}

const char *app_result_string(AppResult result)
{
    switch (result)
    {
    case APP_RESULT_OK:
        return "success";
    case APP_RESULT_INVALID_CONFIG:
        return "invalid application configuration";
    case APP_RESULT_INVALID_BACKEND:
        return "invalid application backend";
    case APP_RESULT_WINDOW_INIT_FAILED:
        return "window initialization failed";
    case APP_RESULT_RESOURCE_LOAD_FAILED:
        return "resource loading failed";
    }

    return "unknown application error";
}

const char *app_state_string(AppState state)
{
    switch (state)
    {
    case APP_STATE_BOOT:
        return "BOOT";
    case APP_STATE_MENU:
        return "MENU";
    case APP_STATE_GAME:
        return "GAME";
    case APP_STATE_EXIT:
        return "EXIT";
    }

    return "UNKNOWN";
}
