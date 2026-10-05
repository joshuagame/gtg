#ifndef GTG_APP_APP_INTERNAL_H
#define GTG_APP_APP_INTERNAL_H

#include "app/app.h"
#include "resources/resources.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    APP_STATE_BOOT = 0,
    APP_STATE_MENU,
    APP_STATE_GAME,
    APP_STATE_EXIT,
} AppState;

typedef struct
{
    bool toggle_pause;
    bool step_once;
    bool start_game;
    bool exit_requested;
} AppInput;

typedef struct
{
    AppState state;
    bool paused;
    uint64_t tick_count;
    unsigned int ticks_last_frame;
    int ticks_per_second;
    int frames_per_second;
    float interpolation_alpha;
} AppView;

typedef struct
{
    void *context;
    void (*configure_window)(void *context, bool resizable, bool fullscreen);
    void (*init_window)(void *context, int width, int height, const char *title);
    bool (*is_window_ready)(void *context);
    void (*set_target_fps)(void *context, int target_fps);
    bool (*window_should_close)(void *context);
    double (*get_frame_time)(void *context);
    int (*get_fps)(void *context);
    AppInput (*read_input)(void *context);
    bool (*load_resources)(void *context, Resources *resources);
    void (*begin_drawing)(void *context);
    void (*clear_background)(void *context);
    void (*draw_overlay)(void *context, const AppView *view);
    void (*end_drawing)(void *context);
    void (*unload_resources)(void *context, Resources *resources);
    void (*close_window)(void *context);
} AppBackend;

AppResult app_run_with_backend(const AppConfig *config, const AppBackend *backend);
const char *app_state_string(AppState state);

#endif
