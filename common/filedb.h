#ifndef _FILEDB_H
#define _FILEDB_H

#include <stddef.h>

typedef struct {
    const char **vec;
    size_t size, capacity;
} server_list_t;

typedef struct {
    const char *name; /* includes path */
    const char *hash;
    size_t size;
    server_list_t *serverlist;
} file_info_t;

typedef struct {
    size_t size, capacity;
    file_info_t *vec;
} filedb_t;

filedb_t* filedb_new();
file_info_t* filedb_insert(filedb_t *db, const char *filename, const char *hash,
    size_t size);
server_list_t* sl_new();
void sl_insert(server_list_t *sl, const char *hostname);
int filedb_scan(filedb_t *db, const char *dirpath);

#endif /* _FILEDB_H */

