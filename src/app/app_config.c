#include "app/app_config.h"

#include <math.h>
#include <stddef.h>

enum
{
    APP_MIN_WINDOW_WIDTH = 320,
    APP_MIN_WINDOW_HEIGHT = 200,
    APP_MAX_WINDOW_DIMENSION = 16384,
    APP_MIN_TARGET_FPS = 1,
    APP_MAX_TARGET_FPS = 1000,
    APP_MIN_SIMULATION_HZ = 1,
    APP_MAX_SIMULATION_HZ = 240,
};

AppConfig app_config_default(void)
{
    return (AppConfig){
        .window_width = 1280,
        .window_height = 720,
        .target_fps = 60,
        .simulation_hz = 30,
        .max_frame_seconds = 0.25,
        .window_title = "GTG - Guerra Termonucleare Globale",
        .resizable = true,
        .fullscreen = false,
        .frame_limit = 0U,
    };
}

bool app_config_is_valid(const AppConfig *config)
{
    if (config == NULL || config->window_title == NULL || config->window_title[0] == '\0')
    {
        return false;
    }

    if (config->window_width < APP_MIN_WINDOW_WIDTH ||
        config->window_width > APP_MAX_WINDOW_DIMENSION)
    {
        return false;
    }

    if (config->window_height < APP_MIN_WINDOW_HEIGHT ||
        config->window_height > APP_MAX_WINDOW_DIMENSION)
    {
        return false;
    }

    if (config->target_fps < APP_MIN_TARGET_FPS || config->target_fps > APP_MAX_TARGET_FPS)
    {
        return false;
    }

    if (config->simulation_hz < APP_MIN_SIMULATION_HZ ||
        config->simulation_hz > APP_MAX_SIMULATION_HZ)
    {
        return false;
    }

    return isfinite(config->max_frame_seconds) && config->max_frame_seconds > 0.0 &&
           config->max_frame_seconds <= 1.0;
}
