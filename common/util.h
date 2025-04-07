#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdint.h>

#define MAX(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) ((a)<(b)?(a):(b))

#define NF_TRY_S(f, fname, a)  if ((f)) { \
    fprintf(stderr, "error: " __FILE__ ":%d: %s()\n", \
        __LINE__, (fname)); \
    a; \
}

#define NF_TRY(f, fname, err, a)  if ((f)) { \
    fprintf(stderr, "error: " __FILE__ ":%d: %s(): %s\n", \
        __LINE__, (fname), (err)); \
    a; \
}

#define NF_TRY_C(f, fname, err, ctx, a)  if ((f)) { \
    fprintf(stderr, "error: " __FILE__ ":%d: %s(): %s on %s\n", \
        __LINE__, (fname), (err), (ctx)); \
    a; \
}

#define DEBUG(c, w) if (debug) fprintf(stderr, c ": " w "\n");
#define DEBUG_VA(c, w, ...) if (debug) fprintf(stderr, c ": " w "\n", __VA_ARGS__);

int is_sha1(const char *str);
void sha1_str2bin(const char *sha1str, uint8_t *sha1bin);
/* sha1str must be at least 41 bytes in size */
void sha1_bin2str(const uint8_t *sha1bin, char *sha1str);

#endif /* _UTIL_H */

