#ifndef GTG_APP_APP_H
#define GTG_APP_APP_H

#include "app/app_config.h"

typedef enum
{
    APP_RESULT_OK = 0,
    APP_RESULT_INVALID_CONFIG,
    APP_RESULT_INVALID_BACKEND,
    APP_RESULT_WINDOW_INIT_FAILED,
    APP_RESULT_RESOURCE_LOAD_FAILED,
} AppResult;

AppResult app_run(const AppConfig *config);
const char *app_result_string(AppResult result);

#endif
