#include <capy/macros.h>

#include "rapidhash/rapidhash.h"

// PUBLIC DEFINITIONS

uint64_t capy_hash(const void *key, uint64_t length)
{
    return rapidhashMicro(key, length);
}
