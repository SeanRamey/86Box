#ifndef UTILITY_H
#define UTILITY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

char* utf8cpy(char* dst, const char* src, size_t sizeDest);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // UTILITY_H
