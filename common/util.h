#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>

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

int is_sha1(const char *str);

#endif /* _UTIL_H */

