#include "app/app_internal.h"

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    AppConfig config;
    Resources resources;
    bool window_initialized;
    bool resources_loaded;
    uint32_t frames_drawn;
} App;

static bool app_backend_is_valid(const AppBackend *backend)
{
    return backend != NULL && backend->configure_window != NULL && backend->init_window != NULL &&
           backend->is_window_ready != NULL && backend->set_target_fps != NULL &&
           backend->window_should_close != NULL && backend->load_resources != NULL &&
           backend->begin_drawing != NULL && backend->clear_background != NULL &&
           backend->end_drawing != NULL && backend->unload_resources != NULL &&
           backend->close_window != NULL;
}

static bool app_frame_limit_reached(const App *app)
{
    return app->config.frame_limit != 0U && app->frames_drawn >= app->config.frame_limit;
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
        backend->begin_drawing(backend->context);
        backend->clear_background(backend->context);
        backend->end_drawing(backend->context);
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
