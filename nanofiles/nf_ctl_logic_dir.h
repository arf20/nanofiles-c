#ifndef _NF_CTL_LOGIC_P2P_H
#define _NF_CTL_LOGIC_P2P_H

#include "../common/filedb.h"
#include "directory_connector.h"

typedef struct {
    const char *directory_hostname;
    dc_t *dc;
} logicdir_t;

logicdir_t* logicdir_new(const char *directory_hostname);
int logicdir_test(logicdir_t *ld);
int logicdir_ping(logicdir_t *ld);
int logicdir_fetch_print_files(logicdir_t *ld);
int logicdir_register_server(logicdir_t *ld, filedb_t *db);
void logicdir_destroy(logicdir_t *ld);

#endif /* _NF_CTL_LOGIC_P2P_H */

