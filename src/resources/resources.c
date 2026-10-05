#include "resources/resources.h"

#include <stddef.h>

bool resources_load(Resources *resources)
{
    if (resources == NULL || resources->loaded)
    {
        return false;
    }

    resources->loaded = true;
    return true;
}

void resources_unload(Resources *resources)
{
    if (resources == NULL)
    {
        return;
    }

    resources->loaded = false;
}
