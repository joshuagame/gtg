#ifndef GTG_APP_APP_CONFIG_H
#define GTG_APP_APP_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int window_width;
    int window_height;
    int target_fps;
    const char *window_title;
    bool resizable;
    bool fullscreen;
    uint32_t frame_limit;
} AppConfig;

AppConfig app_config_default(void);
bool app_config_is_valid(const AppConfig *config);

#endif
