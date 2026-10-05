#include "world/world.h"

#include <assert.h>
#include <math.h>
#include <string.h>

_Static_assert(sizeof(FactionId) == sizeof(uint16_t), "FactionId must remain compact");
_Static_assert(sizeof(RegionId) == sizeof(uint16_t), "RegionId must remain compact");
_Static_assert(sizeof(TargetId) == sizeof(uint16_t), "TargetId must remain compact");
_Static_assert(sizeof(PlatformId) == sizeof(uint16_t), "PlatformId must remain compact");
_Static_assert(sizeof(StrikeId) == sizeof(uint16_t), "StrikeId must remain compact");
_Static_assert(WORLD_FACTION_CAPACITY <= UINT16_MAX, "FactionId cannot represent every slot");
_Static_assert(WORLD_REGION_CAPACITY <= UINT16_MAX, "RegionId cannot represent every slot");
_Static_assert(WORLD_TARGET_CAPACITY <= UINT16_MAX, "TargetId cannot represent every slot");
_Static_assert(WORLD_PLATFORM_CAPACITY <= UINT16_MAX, "PlatformId cannot represent every slot");
_Static_assert(WORLD_STRIKE_CAPACITY <= UINT16_MAX, "StrikeId cannot represent every slot");

static const Faction study_factions[] = {
    {
        .population_initial = UINT64_C(320000000),
        .population_remaining = UINT64_C(320000000),
        .id = {0U},
        .command_points = 8U,
        .command_integrity = 100U,
        .infrastructure_integrity = 100U,
        .name = "OCCIDENTE",
    },
    {
        .population_initial = UINT64_C(290000000),
        .population_remaining = UINT64_C(290000000),
        .id = {1U},
        .command_points = 8U,
        .command_integrity = 100U,
        .infrastructure_integrity = 100U,
        .name = "ORIENTE",
    },
};

static const Region study_regions[] = {
    {{0U}, {0U}, {0.20F, 0.28F}, 100U, 82U, "OCCIDENTE NORD"},
    {{1U}, {0U}, {0.27F, 0.66F}, 100U, 68U, "OCCIDENTE SUD"},
    {{2U}, {1U}, {0.76F, 0.30F}, 100U, 80U, "ORIENTE NORD"},
    {{3U}, {1U}, {0.70F, 0.68F}, 100U, 65U, "ORIENTE SUD"},
};

static const Target study_targets[] = {
    {UINT64_C(12000000),
     {0U},
     {0U},
     {0U},
     {0.18F, 0.25F},
     TARGET_CATEGORY_CIVILIAN,
     100U,
     65U,
     "AURORA"},
    {0U, {1U}, {0U}, {0U}, {0.23F, 0.31F}, TARGET_CATEGORY_COMMAND, 100U, 95U, "CITADEL WEST"},
    {UINT64_C(250000),
     {2U},
     {0U},
     {1U},
     {0.29F, 0.64F},
     TARGET_CATEGORY_INFRASTRUCTURE,
     100U,
     72U,
     "PACIFIC GRID"},
    {UINT64_C(11000000),
     {3U},
     {1U},
     {2U},
     {0.78F, 0.27F},
     TARGET_CATEGORY_CIVILIAN,
     100U,
     64U,
     "VOLNA"},
    {0U, {4U}, {1U}, {2U}, {0.73F, 0.33F}, TARGET_CATEGORY_COMMAND, 100U, 95U, "CITADEL EAST"},
    {UINT64_C(220000),
     {5U},
     {1U},
     {3U},
     {0.68F, 0.65F},
     TARGET_CATEGORY_INFRASTRUCTURE,
     100U,
     70U,
     "STEPPE GRID"},
};

static const Platform study_platforms[] = {
    {{0U},
     {0U},
     {0U},
     {0.16F, 0.34F},
     PLATFORM_CATEGORY_SILO,
     PLATFORM_STATUS_READY,
     4U,
     100U,
     "ATLAS-1"},
    {{1U},
     {0U},
     {1U},
     {0.31F, 0.70F},
     PLATFORM_CATEGORY_BOMBER,
     PLATFORM_STATUS_READY,
     2U,
     85U,
     "EAGLE-2"},
    {{2U},
     {0U},
     {1U},
     {0.39F, 0.78F},
     PLATFORM_CATEGORY_SUBMARINE,
     PLATFORM_STATUS_DEPLOYED,
     6U,
     92U,
     "TRIDENT-3"},
    {{3U},
     {1U},
     {2U},
     {0.81F, 0.35F},
     PLATFORM_CATEGORY_SILO,
     PLATFORM_STATUS_READY,
     4U,
     100U,
     "ZENITH-1"},
    {{4U},
     {1U},
     {3U},
     {0.66F, 0.72F},
     PLATFORM_CATEGORY_BOMBER,
     PLATFORM_STATUS_READY,
     2U,
     84U,
     "VOSTOK-2"},
    {{5U},
     {1U},
     {3U},
     {0.60F, 0.79F},
     PLATFORM_CATEGORY_SUBMARINE,
     PLATFORM_STATUS_DEPLOYED,
     6U,
     91U,
     "TYPHOON-3"},
};

static const Strike study_strikes[] = {
    {{0U}, {0U}, {0U}, {4U}, {0.16F, 0.34F}, {0.73F, 0.33F}, STRIKE_STATUS_IN_FLIGHT, 18U},
    {{1U}, {1U}, {3U}, {1U}, {0.81F, 0.35F}, {0.23F, 0.31F}, STRIKE_STATUS_IN_FLIGHT, 20U},
};

_Static_assert(sizeof(study_factions) / sizeof(study_factions[0]) <= WORLD_FACTION_CAPACITY,
               "study fixture exceeds faction capacity");
_Static_assert(sizeof(study_regions) / sizeof(study_regions[0]) <= WORLD_REGION_CAPACITY,
               "study fixture exceeds region capacity");
_Static_assert(sizeof(study_targets) / sizeof(study_targets[0]) <= WORLD_TARGET_CAPACITY,
               "study fixture exceeds target capacity");
_Static_assert(sizeof(study_platforms) / sizeof(study_platforms[0]) <= WORLD_PLATFORM_CAPACITY,
               "study fixture exceeds platform capacity");
_Static_assert(sizeof(study_strikes) / sizeof(study_strikes[0]) <= WORLD_STRIKE_CAPACITY,
               "study fixture exceeds strike capacity");

static bool world_name_is_valid(const char name[WORLD_NAME_CAPACITY])
{
    return name[0] != '\0' && memchr(name, '\0', WORLD_NAME_CAPACITY) != NULL;
}

static bool world_position_is_valid(WorldPosition position)
{
    return isfinite(position.x) && isfinite(position.y) && position.x >= 0.0F &&
           position.x <= 1.0F && position.y >= 0.0F && position.y <= 1.0F;
}

void world_init(World *world)
{
    if (world == NULL)
    {
        return;
    }

    *world = (World){0};
    world->defcon = WORLD_DEFCON_MAX;
}

bool world_create_study_fixture(World *world)
{
    if (world == NULL)
    {
        return false;
    }

    world_init(world);
    (void)memcpy(world->factions, study_factions, sizeof(study_factions));
    (void)memcpy(world->regions, study_regions, sizeof(study_regions));
    (void)memcpy(world->targets, study_targets, sizeof(study_targets));
    (void)memcpy(world->platforms, study_platforms, sizeof(study_platforms));
    (void)memcpy(world->strikes, study_strikes, sizeof(study_strikes));
    world->faction_count = sizeof(study_factions) / sizeof(study_factions[0]);
    world->region_count = sizeof(study_regions) / sizeof(study_regions[0]);
    world->target_count = sizeof(study_targets) / sizeof(study_targets[0]);
    world->platform_count = sizeof(study_platforms) / sizeof(study_platforms[0]);
    world->strike_count = sizeof(study_strikes) / sizeof(study_strikes[0]);
    world->turn = 1U;
    world->defcon = 3U;

    world_assert_valid(world);
    return world_validate(world) == WORLD_VALIDATION_OK;
}

bool world_faction_id_is_valid(const World *world, FactionId id)
{
    return world != NULL && world->faction_count <= WORLD_FACTION_CAPACITY &&
           (size_t)id.value < world->faction_count &&
           world->factions[id.value].id.value == id.value;
}

bool world_region_id_is_valid(const World *world, RegionId id)
{
    return world != NULL && world->region_count <= WORLD_REGION_CAPACITY &&
           (size_t)id.value < world->region_count && world->regions[id.value].id.value == id.value;
}

bool world_target_id_is_valid(const World *world, TargetId id)
{
    return world != NULL && world->target_count <= WORLD_TARGET_CAPACITY &&
           (size_t)id.value < world->target_count && world->targets[id.value].id.value == id.value;
}

bool world_platform_id_is_valid(const World *world, PlatformId id)
{
    return world != NULL && world->platform_count <= WORLD_PLATFORM_CAPACITY &&
           (size_t)id.value < world->platform_count &&
           world->platforms[id.value].id.value == id.value;
}

bool world_strike_id_is_valid(const World *world, StrikeId id)
{
    return world != NULL && world->strike_count <= WORLD_STRIKE_CAPACITY &&
           (size_t)id.value < world->strike_count && world->strikes[id.value].id.value == id.value;
}

const Faction *world_faction(const World *world, FactionId id)
{
    return world_faction_id_is_valid(world, id) ? &world->factions[id.value] : NULL;
}

const Region *world_region(const World *world, RegionId id)
{
    return world_region_id_is_valid(world, id) ? &world->regions[id.value] : NULL;
}

const Target *world_target(const World *world, TargetId id)
{
    return world_target_id_is_valid(world, id) ? &world->targets[id.value] : NULL;
}

const Platform *world_platform(const World *world, PlatformId id)
{
    return world_platform_id_is_valid(world, id) ? &world->platforms[id.value] : NULL;
}

const Strike *world_strike(const World *world, StrikeId id)
{
    return world_strike_id_is_valid(world, id) ? &world->strikes[id.value] : NULL;
}

WorldValidationResult world_validate(const World *world)
{
    if (world == NULL)
    {
        return WORLD_VALIDATION_NULL;
    }
    if (world->faction_count > WORLD_FACTION_CAPACITY ||
        world->region_count > WORLD_REGION_CAPACITY ||
        world->target_count > WORLD_TARGET_CAPACITY ||
        world->platform_count > WORLD_PLATFORM_CAPACITY ||
        world->strike_count > WORLD_STRIKE_CAPACITY)
    {
        return WORLD_VALIDATION_COUNT;
    }
    if (world->defcon < WORLD_DEFCON_MIN || world->defcon > WORLD_DEFCON_MAX)
    {
        return WORLD_VALIDATION_DEFCON;
    }

    for (size_t index = 0U; index < world->faction_count; index += 1U)
    {
        const Faction *faction = &world->factions[index];
        if (faction->id.value != index || !world_name_is_valid(faction->name) ||
            faction->population_remaining > faction->population_initial ||
            faction->command_integrity > WORLD_PERCENT_MAX ||
            faction->infrastructure_integrity > WORLD_PERCENT_MAX)
        {
            return WORLD_VALIDATION_FACTION;
        }
    }

    for (size_t index = 0U; index < world->region_count; index += 1U)
    {
        const Region *region = &world->regions[index];
        if (region->id.value != index || !world_faction_id_is_valid(world, region->faction_id) ||
            !world_position_is_valid(region->position) || !world_name_is_valid(region->name) ||
            region->infrastructure_integrity > WORLD_PERCENT_MAX ||
            region->detection_coverage > WORLD_PERCENT_MAX)
        {
            return WORLD_VALIDATION_REGION;
        }
    }

    for (size_t index = 0U; index < world->target_count; index += 1U)
    {
        const Target *target = &world->targets[index];
        const Region *region = world_region(world, target->region_id);
        if (target->id.value != index || !world_faction_id_is_valid(world, target->faction_id) ||
            region == NULL || region->faction_id.value != target->faction_id.value ||
            !world_position_is_valid(target->position) || !world_name_is_valid(target->name) ||
            (unsigned int)target->category >= (unsigned int)TARGET_CATEGORY_COUNT ||
            target->integrity > WORLD_PERCENT_MAX || target->strategic_value > WORLD_PERCENT_MAX)
        {
            return WORLD_VALIDATION_TARGET;
        }
    }

    for (size_t index = 0U; index < world->platform_count; index += 1U)
    {
        const Platform *platform = &world->platforms[index];
        const Region *region = world_region(world, platform->region_id);
        if (platform->id.value != index ||
            !world_faction_id_is_valid(world, platform->faction_id) || region == NULL ||
            region->faction_id.value != platform->faction_id.value ||
            !world_position_is_valid(platform->position) || !world_name_is_valid(platform->name) ||
            (unsigned int)platform->category >= (unsigned int)PLATFORM_CATEGORY_COUNT ||
            (unsigned int)platform->status >= (unsigned int)PLATFORM_STATUS_COUNT ||
            platform->readiness > WORLD_PERCENT_MAX)
        {
            return WORLD_VALIDATION_PLATFORM;
        }
    }

    for (size_t index = 0U; index < world->strike_count; index += 1U)
    {
        const Strike *strike = &world->strikes[index];
        const Platform *platform = world_platform(world, strike->platform_id);
        const Target *target = world_target(world, strike->target_id);
        if (strike->id.value != index || !world_faction_id_is_valid(world, strike->faction_id) ||
            platform == NULL || target == NULL ||
            platform->faction_id.value != strike->faction_id.value ||
            target->faction_id.value == strike->faction_id.value ||
            !world_position_is_valid(strike->origin) ||
            !world_position_is_valid(strike->destination) ||
            (unsigned int)strike->status >= (unsigned int)STRIKE_STATUS_COUNT ||
            (strike->status == STRIKE_STATUS_IN_FLIGHT && strike->ticks_remaining == 0U))
        {
            return WORLD_VALIDATION_STRIKE;
        }
    }

    return WORLD_VALIDATION_OK;
}

void world_assert_valid(const World *world)
{
    (void)world;
    assert(world_validate(world) == WORLD_VALIDATION_OK);
}

const char *world_validation_result_string(WorldValidationResult result)
{
    switch (result)
    {
    case WORLD_VALIDATION_OK:
        return "valid";
    case WORLD_VALIDATION_NULL:
        return "null world";
    case WORLD_VALIDATION_COUNT:
        return "capacity exceeded";
    case WORLD_VALIDATION_DEFCON:
        return "invalid DEFCON";
    case WORLD_VALIDATION_FACTION:
        return "invalid faction";
    case WORLD_VALIDATION_REGION:
        return "invalid region";
    case WORLD_VALIDATION_TARGET:
        return "invalid target";
    case WORLD_VALIDATION_PLATFORM:
        return "invalid platform";
    case WORLD_VALIDATION_STRIKE:
        return "invalid strike";
    }
    return "unknown validation result";
}
