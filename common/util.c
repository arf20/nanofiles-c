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

void
sha1_str2bin(const char *sha1str, char *sha1bin)
{
    for (size_t i = 0; i < 20; i++) {
        sscanf(sha1str, "%2hhx", &sha1bin[i]);
        sha1str += 2;
    }  
}

void
sha1_bin2str(const char *sha1bin, char *sha1str)
{
    for (size_t i = 0; i < 20; i++, sha1str += 2)
        sprintf(sha1str, "%02x", sha1bin[i]);
    sha1str[40] = '\0';
}

