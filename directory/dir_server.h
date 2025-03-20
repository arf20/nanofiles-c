#ifndef _DIR_SERVER_H
#define _DIR_SERVER_H

#include "../common/filedb.h"

typedef struct {
    filedb_t *db;
    float corruption_prob;
    int sock;
} dir_server_t;

dir_server_t* ds_new(float corruption_prob);
void ds_run(dir_server_t *ds);


#endif /* _DIR_SERVER_H */

