#ifndef CAPY_STD_H
#define CAPY_STD_H

#ifdef __GNUC__
#define must_check __attribute__((warn_unused_result))
#define unused __attribute__((unused))
#define nofail (void)!
#else
#define unused
#define must_check
#define nofail
#endif

#include <inttypes.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#endif
