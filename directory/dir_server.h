#ifndef _DIR_SERVER_H
#define _DIR_SERVER_H

#include "../common/filedb.h"

typedef struct {
    filedb_t *db;
    float corruption_prob;
} dir_server_t;

dir_server_t* dir_server_new(float corruption_prob);

#endif /* _DIR_SERVER_H */

