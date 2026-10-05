#ifndef GTG_WORLD_WORLD_H
#define GTG_WORLD_WORLD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum
{
    WORLD_NAME_CAPACITY = 32,
    WORLD_FACTION_CAPACITY = 2,
    WORLD_REGION_CAPACITY = 16,
    WORLD_TARGET_CAPACITY = 32,
    WORLD_PLATFORM_CAPACITY = 16,
    WORLD_STRIKE_CAPACITY = 32,
    WORLD_PERCENT_MAX = 100,
    WORLD_DEFCON_MIN = 1,
    WORLD_DEFCON_MAX = 5,
};

typedef struct
{
    uint16_t value;
} FactionId;

typedef struct
{
    uint16_t value;
} RegionId;

typedef struct
{
    uint16_t value;
} TargetId;

typedef struct
{
    uint16_t value;
} PlatformId;

typedef struct
{
    uint16_t value;
} StrikeId;

typedef struct
{
    float x;
    float y;
} WorldPosition;

typedef enum
{
    TARGET_CATEGORY_CIVILIAN = 0,
    TARGET_CATEGORY_COMMAND,
    TARGET_CATEGORY_INFRASTRUCTURE,
    TARGET_CATEGORY_MILITARY,
    TARGET_CATEGORY_COUNT,
} TargetCategory;

typedef enum
{
    PLATFORM_CATEGORY_SILO = 0,
    PLATFORM_CATEGORY_BOMBER,
    PLATFORM_CATEGORY_SUBMARINE,
    PLATFORM_CATEGORY_COUNT,
} PlatformCategory;

typedef enum
{
    PLATFORM_STATUS_READY = 0,
    PLATFORM_STATUS_PREPARING,
    PLATFORM_STATUS_DEPLOYED,
    PLATFORM_STATUS_DESTROYED,
    PLATFORM_STATUS_COUNT,
} PlatformStatus;

typedef enum
{
    STRIKE_STATUS_PLANNED = 0,
    STRIKE_STATUS_IN_FLIGHT,
    STRIKE_STATUS_INTERCEPTED,
    STRIKE_STATUS_IMPACTED,
    STRIKE_STATUS_COUNT,
} StrikeStatus;

typedef struct
{
    uint64_t population_initial;
    uint64_t population_remaining;
    FactionId id;
    uint16_t command_points;
    uint8_t command_integrity;
    uint8_t infrastructure_integrity;
    char name[WORLD_NAME_CAPACITY];
} Faction;

typedef struct
{
    RegionId id;
    FactionId faction_id;
    WorldPosition position;
    uint8_t infrastructure_integrity;
    uint8_t detection_coverage;
    char name[WORLD_NAME_CAPACITY];
} Region;

typedef struct
{
    uint64_t population;
    TargetId id;
    FactionId faction_id;
    RegionId region_id;
    WorldPosition position;
    TargetCategory category;
    uint8_t integrity;
    uint8_t strategic_value;
    char name[WORLD_NAME_CAPACITY];
} Target;

typedef struct
{
    PlatformId id;
    FactionId faction_id;
    RegionId region_id;
    WorldPosition position;
    PlatformCategory category;
    PlatformStatus status;
    uint16_t payloads_available;
    uint8_t readiness;
    char name[WORLD_NAME_CAPACITY];
} Platform;

typedef struct
{
    StrikeId id;
    FactionId faction_id;
    PlatformId platform_id;
    TargetId target_id;
    WorldPosition origin;
    WorldPosition destination;
    StrikeStatus status;
    uint32_t ticks_remaining;
} Strike;

typedef struct
{
    Faction factions[WORLD_FACTION_CAPACITY];
    Region regions[WORLD_REGION_CAPACITY];
    Target targets[WORLD_TARGET_CAPACITY];
    Platform platforms[WORLD_PLATFORM_CAPACITY];
    Strike strikes[WORLD_STRIKE_CAPACITY];
    size_t faction_count;
    size_t region_count;
    size_t target_count;
    size_t platform_count;
    size_t strike_count;
    uint32_t turn;
    uint8_t defcon;
} World;

typedef enum
{
    WORLD_VALIDATION_OK = 0,
    WORLD_VALIDATION_NULL,
    WORLD_VALIDATION_COUNT,
    WORLD_VALIDATION_DEFCON,
    WORLD_VALIDATION_FACTION,
    WORLD_VALIDATION_REGION,
    WORLD_VALIDATION_TARGET,
    WORLD_VALIDATION_PLATFORM,
    WORLD_VALIDATION_STRIKE,
} WorldValidationResult;

/** Initializes an empty world with valid global defaults. */
void world_init(World *world);

/** Builds a small two-faction fixture intended for study and unit tests. */
bool world_create_study_fixture(World *world);

/** Validates counts, IDs, references, ranges, names, and normalized positions. */
WorldValidationResult world_validate(const World *world);

/** Asserts world validity when assertions are enabled; has no effect in release builds. */
void world_assert_valid(const World *world);

const char *world_validation_result_string(WorldValidationResult result);

bool world_faction_id_is_valid(const World *world, FactionId id);
bool world_region_id_is_valid(const World *world, RegionId id);
bool world_target_id_is_valid(const World *world, TargetId id);
bool world_platform_id_is_valid(const World *world, PlatformId id);
bool world_strike_id_is_valid(const World *world, StrikeId id);

const Faction *world_faction(const World *world, FactionId id);
const Region *world_region(const World *world, RegionId id);
const Target *world_target(const World *world, TargetId id);
const Platform *world_platform(const World *world, PlatformId id);
const Strike *world_strike(const World *world, StrikeId id);

#endif
