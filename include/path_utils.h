/* Small header for mini-lib used for path parsing */

#pragma once

#include <stddef.h>

#ifndef MAX_NAME_LEN
#define MAX_NAME_LEN     32
#endif

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN     256
#endif

typedef char filename_t[MAX_NAME_LEN];
typedef char path_t[MAX_PATH_LEN];

char *UtilsParsePathGetNext(const char *path, path_t buf, char **state);
int UtilsParsePath(const char *path, filename_t entryList[], size_t entryListLen);
