#include <rapidhash/rapidhash.h>

#include "capy.h"

// PUBLIC DEFINITIONS

uint64_t capy_hash(const void *key, uint64_t length)
{
    return rapidhashMicro(key, length);
}
