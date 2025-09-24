#include <capy/capy.h>
#include <rapidhash/rapidhash.h>

uint64_t capy_hash(const void *key, uint64_t length)
{
    return rapidhashMicro(key, length);
}
