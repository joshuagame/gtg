#include "app/app_internal.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum
{
    FAKE_INPUT_CAPACITY = 128,
};

typedef struct
{
    unsigned int configure_calls;
    unsigned int init_calls;
    unsigned int ready_calls;
    unsigned int target_fps_calls;
    unsigned int should_close_calls;
    unsigned int frame_time_calls;
    unsigned int fps_calls;
    unsigned int input_calls;
    unsigned int load_calls;
    unsigned int begin_calls;
    unsigned int clear_calls;
    unsigned int overlay_calls;
    unsigned int end_calls;
    unsigned int unload_calls;
    unsigned int close_calls;
    bool window_ready;
    bool resource_load_succeeds;
    double frame_seconds;
    int frames_per_second;
    AppInput inputs[FAKE_INPUT_CAPACITY];
    size_t input_count;
    size_t input_index;
    AppView last_view;
} FakeRuntime;

static int failures = 0;

#define EXPECT_TRUE(condition) test_expect_true((condition), #condition, __LINE__)
#define EXPECT_EQ_UINT(expected, actual)                                                           \
    test_expect_equal_uint((expected), (actual), #actual, __LINE__)
#define EXPECT_EQ_U64(expected, actual)                                                            \
    test_expect_equal_u64((expected), (actual), #actual, __LINE__)
#define EXPECT_EQ_RESULT(expected, actual)                                                         \
    test_expect_equal_result((expected), (actual), #actual, __LINE__)
#define EXPECT_EQ_STATE(expected, actual)                                                          \
    test_expect_equal_state((expected), (actual), #actual, __LINE__)
#define EXPECT_NEAR(expected, actual, tolerance)                                                   \
    test_expect_near((expected), (actual), (tolerance), #actual, __LINE__)

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

static void test_expect_equal_u64(uint64_t expected, uint64_t actual, const char *expression,
                                  int line)
{
    if (expected != actual)
    {
        fprintf(stderr, "FAIL line %d: expected %s to be %" PRIu64 ", got %" PRIu64 "\n", line,
                expression, expected, actual);
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

static void test_expect_equal_state(AppState expected, AppState actual, const char *expression,
                                    int line)
{
    if (expected != actual)
    {
        fprintf(stderr, "FAIL line %d: expected %s to be %s, got %s\n", line, expression,
                app_state_string(expected), app_state_string(actual));
        failures += 1;
    }
}

static void test_expect_near(double expected, double actual, double tolerance,
                             const char *expression, int line)
{
    const double difference = expected > actual ? expected - actual : actual - expected;

    if (difference > tolerance)
    {
        fprintf(stderr, "FAIL line %d: expected %s near %.6f, got %.6f\n", line, expression,
                expected, actual);
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

static double fake_get_frame_time(void *context)
{
    FakeRuntime *runtime = context;
    runtime->frame_time_calls += 1U;
    return runtime->frame_seconds;
}

static int fake_get_fps(void *context)
{
    FakeRuntime *runtime = context;
    runtime->fps_calls += 1U;
    return runtime->frames_per_second;
}

static AppInput fake_read_input(void *context)
{
    FakeRuntime *runtime = context;
    AppInput input = {0};
    runtime->input_calls += 1U;

    if (runtime->input_index < runtime->input_count)
    {
        input = runtime->inputs[runtime->input_index];
    }
    runtime->input_index += 1U;
    return input;
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

static void fake_draw_overlay(void *context, const AppView *view)
{
    FakeRuntime *runtime = context;
    runtime->overlay_calls += 1U;
    runtime->last_view = *view;
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
        .get_frame_time = fake_get_frame_time,
        .get_fps = fake_get_fps,
        .read_input = fake_read_input,
        .load_resources = fake_load_resources,
        .begin_drawing = fake_begin_drawing,
        .clear_background = fake_clear_background,
        .draw_overlay = fake_draw_overlay,
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
        .frame_seconds = 1.0 / 60.0,
        .frames_per_second = 60,
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
    EXPECT_EQ_UINT(30U, (unsigned int)config.simulation_hz);
    EXPECT_NEAR(0.25, config.max_frame_seconds, 1.0e-12);
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
        EXPECT_EQ_UINT(2U, runtime.frame_time_calls);
        EXPECT_EQ_UINT(2U, runtime.input_calls);
        EXPECT_EQ_UINT(2U, runtime.begin_calls);
        EXPECT_EQ_UINT(2U, runtime.clear_calls);
        EXPECT_EQ_UINT(2U, runtime.overlay_calls);
        EXPECT_EQ_UINT(2U, runtime.end_calls);
        EXPECT_EQ_UINT(1U, runtime.unload_calls);
        EXPECT_EQ_UINT(1U, runtime.close_calls);
        EXPECT_EQ_U64(1U, runtime.last_view.tick_count);
        EXPECT_EQ_STATE(APP_STATE_MENU, runtime.last_view.state);
    }
}

static uint64_t run_for_one_second(uint32_t frame_count, double frame_seconds)
{
    AppConfig config = app_config_default();
    FakeRuntime runtime = fake_runtime_default();
    const AppBackend backend = fake_backend(&runtime);
    config.frame_limit = frame_count;
    runtime.frame_seconds = frame_seconds;

    EXPECT_EQ_RESULT(APP_RESULT_OK, app_run_with_backend(&config, &backend));
    return runtime.last_view.tick_count;
}

static void test_tick_count_is_independent_from_frame_rate(void)
{
    const uint64_t ticks_at_60_fps = run_for_one_second(60U, 1.0 / 60.0);
    const uint64_t ticks_at_30_fps = run_for_one_second(30U, 1.0 / 30.0);

    EXPECT_EQ_U64(30U, ticks_at_60_fps);
    EXPECT_EQ_U64(30U, ticks_at_30_fps);
}

static void test_pause_keeps_input_and_rendering_responsive(void)
{
    AppConfig config = app_config_default();
    FakeRuntime runtime = fake_runtime_default();
    const AppBackend backend = fake_backend(&runtime);
    config.frame_limit = 4U;
    runtime.frame_seconds = 1.0 / 30.0;
    runtime.input_count = 1U;
    runtime.inputs[0].toggle_pause = true;

    EXPECT_EQ_RESULT(APP_RESULT_OK, app_run_with_backend(&config, &backend));
    EXPECT_EQ_U64(0U, runtime.last_view.tick_count);
    EXPECT_TRUE(runtime.last_view.paused);
    EXPECT_EQ_UINT(0U, (unsigned int)runtime.last_view.ticks_per_second);
    EXPECT_EQ_UINT(4U, runtime.input_calls);
    EXPECT_EQ_UINT(4U, runtime.overlay_calls);
}

static void test_single_step_advances_exactly_one_tick_while_paused(void)
{
    AppConfig config = app_config_default();
    FakeRuntime runtime = fake_runtime_default();
    const AppBackend backend = fake_backend(&runtime);
    config.frame_limit = 4U;
    runtime.frame_seconds = 1.0 / 30.0;
    runtime.input_count = 2U;
    runtime.inputs[0].toggle_pause = true;
    runtime.inputs[1].step_once = true;

    EXPECT_EQ_RESULT(APP_RESULT_OK, app_run_with_backend(&config, &backend));
    EXPECT_EQ_U64(1U, runtime.last_view.tick_count);
    EXPECT_EQ_STATE(APP_STATE_MENU, runtime.last_view.state);
    EXPECT_TRUE(runtime.last_view.paused);
}

static void test_state_machine_reaches_game(void)
{
    AppConfig config = app_config_default();
    FakeRuntime runtime = fake_runtime_default();
    const AppBackend backend = fake_backend(&runtime);
    config.frame_limit = 2U;
    runtime.frame_seconds = 1.0 / 30.0;
    runtime.input_count = 2U;
    runtime.inputs[1].start_game = true;

    EXPECT_EQ_RESULT(APP_RESULT_OK, app_run_with_backend(&config, &backend));
    EXPECT_EQ_U64(2U, runtime.last_view.tick_count);
    EXPECT_EQ_STATE(APP_STATE_GAME, runtime.last_view.state);
}

static void test_long_frame_is_clamped(void)
{
    AppConfig config = app_config_default();
    FakeRuntime runtime = fake_runtime_default();
    const AppBackend backend = fake_backend(&runtime);
    config.frame_limit = 1U;
    runtime.frame_seconds = 5.0;

    EXPECT_EQ_RESULT(APP_RESULT_OK, app_run_with_backend(&config, &backend));
    EXPECT_EQ_U64(7U, runtime.last_view.tick_count);
    EXPECT_EQ_UINT(7U, runtime.last_view.ticks_last_frame);
    EXPECT_NEAR(0.5, (double)runtime.last_view.interpolation_alpha, 1.0e-6);
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
    test_tick_count_is_independent_from_frame_rate();
    test_pause_keeps_input_and_rendering_responsive();
    test_single_step_advances_exactly_one_tick_while_paused();
    test_state_machine_reaches_game();
    test_long_frame_is_clamped();
    test_resource_failure_closes_window();
    test_window_failure_uses_cleanup_path();

    if (failures != 0)
    {
        fprintf(stderr, "FAIL: %d assertion(s) failed\n", failures);
        return 1;
    }

    puts("PASS: GTG fixed-timestep and lifecycle tests");
    return 0;
}
