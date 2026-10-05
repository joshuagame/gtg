#include "world/world.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

#define EXPECT_TRUE(condition) test_expect_true((condition), #condition, __LINE__)
#define EXPECT_EQ_SIZE(expected, actual)                                                           \
    test_expect_equal_size((expected), (actual), #actual, __LINE__)
#define EXPECT_EQ_VALIDATION(expected, actual)                                                     \
    test_expect_validation((expected), (actual), #actual, __LINE__)

static void test_expect_true(bool condition, const char *expression, int line)
{
    if (!condition)
    {
        fprintf(stderr, "FAIL line %d: expected %s\n", line, expression);
        failures += 1;
    }
}

static void test_expect_equal_size(size_t expected, size_t actual, const char *expression, int line)
{
    if (expected != actual)
    {
        fprintf(stderr, "FAIL line %d: expected %s to be %zu, got %zu\n", line, expression,
                expected, actual);
        failures += 1;
    }
}

static void test_expect_validation(WorldValidationResult expected, WorldValidationResult actual,
                                   const char *expression, int line)
{
    if (expected != actual)
    {
        fprintf(stderr, "FAIL line %d: expected %s to be %s, got %s\n", line, expression,
                world_validation_result_string(expected), world_validation_result_string(actual));
        failures += 1;
    }
}

static World study_world(void)
{
    World world;
    EXPECT_TRUE(world_create_study_fixture(&world));
    return world;
}

static void test_empty_world_has_valid_defaults(void)
{
    World world;
    world_init(&world);

    EXPECT_EQ_VALIDATION(WORLD_VALIDATION_OK, world_validate(&world));
    EXPECT_EQ_SIZE(0U, world.faction_count);
    EXPECT_TRUE(world.defcon == WORLD_DEFCON_MAX);
}

static void test_study_fixture_contains_two_opposing_factions(void)
{
    const World world = study_world();
    const Faction *west = world_faction(&world, (FactionId){0U});
    const Faction *east = world_faction(&world, (FactionId){1U});

    EXPECT_EQ_VALIDATION(WORLD_VALIDATION_OK, world_validate(&world));
    EXPECT_EQ_SIZE(2U, world.faction_count);
    EXPECT_EQ_SIZE(4U, world.region_count);
    EXPECT_EQ_SIZE(6U, world.target_count);
    EXPECT_EQ_SIZE(6U, world.platform_count);
    EXPECT_EQ_SIZE(2U, world.strike_count);
    EXPECT_TRUE(west != NULL && strcmp(west->name, "OCCIDENTE") == 0);
    EXPECT_TRUE(east != NULL && strcmp(east->name, "ORIENTE") == 0);
}

static void test_queries_reject_every_invalid_id(void)
{
    World world = study_world();

    EXPECT_TRUE(world_faction(&world, (FactionId){2U}) == NULL);
    EXPECT_TRUE(world_region(&world, (RegionId){4U}) == NULL);
    EXPECT_TRUE(world_target(&world, (TargetId){6U}) == NULL);
    EXPECT_TRUE(world_platform(&world, (PlatformId){6U}) == NULL);
    EXPECT_TRUE(world_strike(&world, (StrikeId){2U}) == NULL);

    world.targets[1].id.value = 0U;
    EXPECT_TRUE(world_target(&world, (TargetId){1U}) == NULL);
}

static void test_all_positions_are_normalized(void)
{
    const World world = study_world();

    for (size_t index = 0U; index < world.region_count; index += 1U)
    {
        const WorldPosition position = world.regions[index].position;
        EXPECT_TRUE(position.x >= 0.0F && position.x <= 1.0F);
        EXPECT_TRUE(position.y >= 0.0F && position.y <= 1.0F);
    }
    for (size_t index = 0U; index < world.target_count; index += 1U)
    {
        const WorldPosition position = world.targets[index].position;
        EXPECT_TRUE(position.x >= 0.0F && position.x <= 1.0F);
        EXPECT_TRUE(position.y >= 0.0F && position.y <= 1.0F);
    }
}

static void test_references_preserve_faction_ownership(void)
{
    const World world = study_world();

    for (size_t index = 0U; index < world.target_count; index += 1U)
    {
        const Target *target = world_target(&world, (TargetId){(uint16_t)index});
        const Region *region = world_region(&world, target->region_id);
        EXPECT_TRUE(region != NULL && region->faction_id.value == target->faction_id.value);
    }

    for (size_t index = 0U; index < world.strike_count; index += 1U)
    {
        const Strike *strike = world_strike(&world, (StrikeId){(uint16_t)index});
        const Platform *platform = world_platform(&world, strike->platform_id);
        const Target *target = world_target(&world, strike->target_id);
        EXPECT_TRUE(platform != NULL && platform->faction_id.value == strike->faction_id.value);
        EXPECT_TRUE(target != NULL && target->faction_id.value != strike->faction_id.value);
    }
}

static void test_validation_reports_broken_invariants(void)
{
    World world = study_world();

    world.target_count = WORLD_TARGET_CAPACITY + 1U;
    EXPECT_EQ_VALIDATION(WORLD_VALIDATION_COUNT, world_validate(&world));

    world = study_world();
    world.defcon = 0U;
    EXPECT_EQ_VALIDATION(WORLD_VALIDATION_DEFCON, world_validate(&world));

    world = study_world();
    world.regions[0].position.x = 1.5F;
    EXPECT_EQ_VALIDATION(WORLD_VALIDATION_REGION, world_validate(&world));

    world = study_world();
    world.targets[0].region_id.value = 3U;
    EXPECT_EQ_VALIDATION(WORLD_VALIDATION_TARGET, world_validate(&world));

    world = study_world();
    world.platforms[0].readiness = 101U;
    EXPECT_EQ_VALIDATION(WORLD_VALIDATION_PLATFORM, world_validate(&world));

    world = study_world();
    world.strikes[0].ticks_remaining = 0U;
    EXPECT_EQ_VALIDATION(WORLD_VALIDATION_STRIKE, world_validate(&world));
}

int main(void)
{
    test_empty_world_has_valid_defaults();
    test_study_fixture_contains_two_opposing_factions();
    test_queries_reject_every_invalid_id();
    test_all_positions_are_normalized();
    test_references_preserve_faction_ownership();
    test_validation_reports_broken_invariants();

    if (failures != 0)
    {
        fprintf(stderr, "FAIL: %d world assertion(s) failed\n", failures);
        return 1;
    }

    puts("PASS: GTG world domain tests");
    return 0;
}
