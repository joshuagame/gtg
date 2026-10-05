#include "app/app.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    AppConfig config = app_config_default();

    if (argc == 2 && strcmp(argv[1], "--smoke-test") == 0)
    {
        config.frame_limit = 1U;
    }
    else if (argc != 1)
    {
        fprintf(stderr, "Usage: %s [--smoke-test]\n", argv[0]);
        return 2;
    }

    const AppResult result = app_run(&config);

    if (result != APP_RESULT_OK)
    {
        fprintf(stderr, "GTG failed: %s\n", app_result_string(result));
        return 1;
    }

    return 0;
}
