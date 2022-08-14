#include <stdio.h>

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_TAG "stable"
#define VERSION_CMD "--version"

#define STR_HELPER(t) # t
#define STR(t) STR_HELPER(t)

#define VERSION STR(VERSION_MAJOR)"."STR(VERSION_MINOR)"."STR(VERSION_PATCH)"-"VERSION_TAG
