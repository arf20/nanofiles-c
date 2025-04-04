#ifndef _NF_CTL_LOGIC_P2P_H
#define _NF_CTL_LOGIC_P2P_H

#include "../common/filedb.h"
#include "directory_connector.h"

typedef struct {
    const char *directory_hostname;
    dc_t *dc;
} logicdir_t;

logicdir_t* logicdir_new(const char *directory_hostname);
int logicdir_test(const logicdir_t *ld);
int logicdir_ping(const logicdir_t *ld);
filedb_t* logicdir_fetch(const logicdir_t *ld);
int logicdir_register_server(const logicdir_t *ld, const filedb_t *db);
void logicdir_destroy(logicdir_t *ld);

#endif /* _NF_CTL_LOGIC_P2P_H */

