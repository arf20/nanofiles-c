#include "util.h"

#include <ctype.h>
#include <string.h>

int
is_sha1(const char *str)
{
    if (strlen(str) != 40)
        return 0;
    while (*str) {
        if (!isxdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

