#ifndef CAPY_STD_H
#define CAPY_STD_H

#if __STDC_VERSION__ <= 199901L
#ifdef __GNUC__
#define _Alignof __alignof__
#endif
#endif

#ifdef __GNUC__
#define must_check __attribute__((warn_unused_result))
#else
#define must_check
#endif

#include <inttypes.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#endif
