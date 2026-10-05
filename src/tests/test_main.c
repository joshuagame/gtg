#include "app/app_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct
{
    unsigned int configure_calls;
    unsigned int init_calls;
    unsigned int ready_calls;
    unsigned int target_fps_calls;
    unsigned int should_close_calls;
    unsigned int load_calls;
    unsigned int begin_calls;
    unsigned int clear_calls;
    unsigned int end_calls;
    unsigned int unload_calls;
    unsigned int close_calls;
    bool window_ready;
    bool resource_load_succeeds;
} FakeRuntime;

static int failures = 0;

#define EXPECT_TRUE(condition) test_expect_true((condition), #condition, __LINE__)
#define EXPECT_EQ_UINT(expected, actual)                                                           \
    test_expect_equal_uint((expected), (actual), #actual, __LINE__)
#define EXPECT_EQ_RESULT(expected, actual)                                                         \
    test_expect_equal_result((expected), (actual), #actual, __LINE__)

static void test_expect_true(bool condition, const char *expression, int line)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL line %d: expected %s\n", line, expression);
        failures += 1;
    }
}

static void test_expect_equal_uint(unsigned int expected, unsigned int actual,
                                   const char *expression, int line)
{
    if (expected != actual)
    {
        fprintf(stderr, "FAIL line %d: expected %s to be %u, got %u\n", line, expression, expected,
                actual);
        failures += 1;
    }
}

static void test_expect_equal_result(AppResult expected, AppResult actual, const char *expression,
                                     int line)
{
    if (expected != actual)
    {
        fprintf(stderr, "FAIL line %d: expected %s to be %s, got %s\n", line, expression,
                app_result_string(expected), app_result_string(actual));
        failures += 1;
    }
}

static void fake_configure_window(void *context, bool resizable, bool fullscreen)
{
    FakeRuntime *runtime = context;
    runtime->configure_calls += 1U;
    EXPECT_TRUE(resizable);
    EXPECT_TRUE(!fullscreen);
}

static void fake_init_window(void *context, int width, int height, const char *title)
{
    FakeRuntime *runtime = context;
    runtime->init_calls += 1U;
    EXPECT_EQ_UINT(1280U, (unsigned int)width);
    EXPECT_EQ_UINT(720U, (unsigned int)height);
    EXPECT_TRUE(title != NULL);
}

static bool fake_is_window_ready(void *context)
{
    FakeRuntime *runtime = context;
    runtime->ready_calls += 1U;
    return runtime->window_ready;
}

static void fake_set_target_fps(void *context, int target_fps)
{
    FakeRuntime *runtime = context;
    runtime->target_fps_calls += 1U;
    EXPECT_EQ_UINT(60U, (unsigned int)target_fps);
}

static bool fake_window_should_close(void *context)
{
    FakeRuntime *runtime = context;
    runtime->should_close_calls += 1U;
    return false;
}

static bool fake_load_resources(void *context, Resources *resources)
{
    FakeRuntime *runtime = context;
    runtime->load_calls += 1U;

    if (!runtime->resource_load_succeeds)
    {
        return false;
    }

    return resources_load(resources);
}

static void fake_begin_drawing(void *context)
{
    FakeRuntime *runtime = context;
    runtime->begin_calls += 1U;
}

static void fake_clear_background(void *context)
{
    FakeRuntime *runtime = context;
    runtime->clear_calls += 1U;
}

static void fake_end_drawing(void *context)
{
    FakeRuntime *runtime = context;
    runtime->end_calls += 1U;
}

static void fake_unload_resources(void *context, Resources *resources)
{
    FakeRuntime *runtime = context;
    runtime->unload_calls += 1U;
    resources_unload(resources);
}

static void fake_close_window(void *context)
{
    FakeRuntime *runtime = context;
    runtime->close_calls += 1U;
}

static AppBackend fake_backend(FakeRuntime *runtime)
{
    return (AppBackend){
        .context = runtime,
        .configure_window = fake_configure_window,
        .init_window = fake_init_window,
        .is_window_ready = fake_is_window_ready,
        .set_target_fps = fake_set_target_fps,
        .window_should_close = fake_window_should_close,
        .load_resources = fake_load_resources,
        .begin_drawing = fake_begin_drawing,
        .clear_background = fake_clear_background,
        .end_drawing = fake_end_drawing,
        .unload_resources = fake_unload_resources,
        .close_window = fake_close_window,
    };
}

static FakeRuntime fake_runtime_default(void)
{
    return (FakeRuntime){
        .window_ready = true,
        .resource_load_succeeds = true,
    };
}

static void test_c17_is_enabled(void)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201710L
    EXPECT_TRUE(false);
#endif
}

static void test_default_config_is_valid(void)
{
    const AppConfig config = app_config_default();

    EXPECT_TRUE(app_config_is_valid(&config));
    EXPECT_EQ_UINT(1280U, (unsigned int)config.window_width);
    EXPECT_EQ_UINT(720U, (unsigned int)config.window_height);
    EXPECT_EQ_UINT(60U, (unsigned int)config.target_fps);
}

static void test_invalid_config_is_rejected_before_initialization(void)
{
    AppConfig config = app_config_default();
    FakeRuntime runtime = fake_runtime_default();
    const AppBackend backend = fake_backend(&runtime);
    config.window_width = 0;

    const AppResult result = app_run_with_backend(&config, &backend);

    EXPECT_EQ_RESULT(APP_RESULT_INVALID_CONFIG, result);
    EXPECT_EQ_UINT(0U, runtime.init_calls);
    EXPECT_EQ_UINT(0U, runtime.close_calls);
}

static void test_successful_lifecycle_can_run_repeatedly(void)
{
    for (unsigned int iteration = 0U; iteration < 3U; iteration += 1U)
    {
        AppConfig config = app_config_default();
        FakeRuntime runtime = fake_runtime_default();
        const AppBackend backend = fake_backend(&runtime);
        config.frame_limit = 2U;

        const AppResult result = app_run_with_backend(&config, &backend);

        EXPECT_EQ_RESULT(APP_RESULT_OK, result);
        EXPECT_EQ_UINT(1U, runtime.configure_calls);
        EXPECT_EQ_UINT(1U, runtime.init_calls);
        EXPECT_EQ_UINT(1U, runtime.ready_calls);
        EXPECT_EQ_UINT(1U, runtime.target_fps_calls);
        EXPECT_EQ_UINT(1U, runtime.load_calls);
        EXPECT_EQ_UINT(2U, runtime.should_close_calls);
        EXPECT_EQ_UINT(2U, runtime.begin_calls);
        EXPECT_EQ_UINT(2U, runtime.clear_calls);
        EXPECT_EQ_UINT(2U, runtime.end_calls);
        EXPECT_EQ_UINT(1U, runtime.unload_calls);
        EXPECT_EQ_UINT(1U, runtime.close_calls);
    }
}

static void test_resource_failure_closes_window(void)
{
    const AppConfig config = app_config_default();
    FakeRuntime runtime = fake_runtime_default();
    const AppBackend backend = fake_backend(&runtime);
    runtime.resource_load_succeeds = false;

    const AppResult result = app_run_with_backend(&config, &backend);

    EXPECT_EQ_RESULT(APP_RESULT_RESOURCE_LOAD_FAILED, result);
    EXPECT_EQ_UINT(1U, runtime.init_calls);
    EXPECT_EQ_UINT(1U, runtime.load_calls);
    EXPECT_EQ_UINT(0U, runtime.unload_calls);
    EXPECT_EQ_UINT(1U, runtime.close_calls);
    EXPECT_EQ_UINT(0U, runtime.begin_calls);
}

static void test_window_failure_uses_cleanup_path(void)
{
    const AppConfig config = app_config_default();
    FakeRuntime runtime = fake_runtime_default();
    const AppBackend backend = fake_backend(&runtime);
    runtime.window_ready = false;

    const AppResult result = app_run_with_backend(&config, &backend);

    EXPECT_EQ_RESULT(APP_RESULT_WINDOW_INIT_FAILED, result);
    EXPECT_EQ_UINT(1U, runtime.init_calls);
    EXPECT_EQ_UINT(0U, runtime.load_calls);
    EXPECT_EQ_UINT(1U, runtime.close_calls);
}

int main(void)
{
    test_c17_is_enabled();
    test_default_config_is_valid();
    test_invalid_config_is_rejected_before_initialization();
    test_successful_lifecycle_can_run_repeatedly();
    test_resource_failure_closes_window();
    test_window_failure_uses_cleanup_path();

    if (failures != 0)
    {
        fprintf(stderr, "FAIL: %d assertion(s) failed\n", failures);
        return 1;
    }

    puts("PASS: GTG application lifecycle tests");
    return 0;
}
