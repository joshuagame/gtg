#ifndef GTG_APP_APP_INTERNAL_H
#define GTG_APP_APP_INTERNAL_H

#include "app/app.h"
#include "resources/resources.h"

#include <stdbool.h>

typedef struct
{
    void *context;
    void (*configure_window)(void *context, bool resizable, bool fullscreen);
    void (*init_window)(void *context, int width, int height, const char *title);
    bool (*is_window_ready)(void *context);
    void (*set_target_fps)(void *context, int target_fps);
    bool (*window_should_close)(void *context);
    bool (*load_resources)(void *context, Resources *resources);
    void (*begin_drawing)(void *context);
    void (*clear_background)(void *context);
    void (*end_drawing)(void *context);
    void (*unload_resources)(void *context, Resources *resources);
    void (*close_window)(void *context);
} AppBackend;

AppResult app_run_with_backend(const AppConfig *config, const AppBackend *backend);

#endif
