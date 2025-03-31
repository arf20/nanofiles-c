#ifndef _NF_CONNECTOR_H
#define _NF_CONNECTOR_H

#include <sys/socket.h>

typedef struct {
    int sock; 
} nfs_t;

nfs_t* nfs_new(unsigned short port);
int nfs_test(nfs_t *nfs);
void nfs_destroy(nfs_t *nfs);

#endif /* _NF_CONNECTOR_H */

