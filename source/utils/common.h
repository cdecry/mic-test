#pragma once

#ifndef COMMON_HEADER
#	define COMMON_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>

[[maybe_unused]] static inline void ok(const char* msg) { printf("\x1b[32m[OK]\x1b[0m  %s\n", msg); }
[[maybe_unused]] static inline void err(const char* msg) { printf("\x1b[31m[!!]\x1b[0m %s\n", msg); }
[[maybe_unused]] static inline void inf(const char* msg) { printf("\x1b[36m[--]\x1b[0m %s\n", msg); }

#endif