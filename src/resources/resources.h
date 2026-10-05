#ifndef GTG_RESOURCES_RESOURCES_H
#define GTG_RESOURCES_RESOURCES_H

#include <stdbool.h>

typedef struct
{
    bool loaded;
} Resources;

bool resources_load(Resources *resources);
void resources_unload(Resources *resources);

#endif
