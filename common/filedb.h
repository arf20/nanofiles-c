#ifndef _FILEDB_H
#define _FILEDB_H

#include <stddef.h>

typedef struct {
    const char *filename; /* includes path */
    const char *hash;
    size_t size;
} file_info_t;

typedef struct {
    size_t size, capacity;
    file_info_t *vec;
} filedb_t;

filedb_t *filedb_new();
int filedb_scan(filedb_t *db, const char *dirpath);

#endif /* _FILEDB_H */

