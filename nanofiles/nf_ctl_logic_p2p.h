#ifndef _NF_CTL_LOGIC_DIR_H
#define _NF_CTL_LOGIC_DIR_H

#include "../common/filedb.h"

#include "nf_server.h"

typedef struct {
    nfs_t *nfs;
} logicp2p_t;

logicp2p_t* logicp2p_new();
int logicp2p_test_client(logicp2p_t *lp, const char *hostname);
int logicp2p_test_server(logicp2p_t *lp);
void logicp2p_start_server(logicp2p_t *lp);
void logicp2p_destroy(logicp2p_t *lp);

#endif /* _NF_CTL_LOGIC_DIR_H */

