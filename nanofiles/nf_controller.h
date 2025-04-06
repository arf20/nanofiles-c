#ifndef _NF_CONTROLLER_H
#define _NF_CONTROLLER_H

#include "../common/filedb.h"

#include "nf_ctl_logic_dir.h"
#include "nf_ctl_logic_p2p.h"
#include "nf_shell.h"

typedef enum { OFFLINE, ONLINE } status_t;

typedef struct {
    const char *directory_hostname;
    const char *shared_dir;
    status_t state;
    logicdir_t *ld;
    logicp2p_t *lp;
    filedb_t *db;
    shell_t *shell;
    int quit;
} ctl_t;

ctl_t* ctl_new(filedb_t *db, const char *directory_hostname,
    const char *shared_dir);
void ctl_test_directory(ctl_t *ctl);
void ctl_process_command(ctl_t *ctl, int test_mode_tcp);
int ctl_should_quit(ctl_t *ctl);
void ctl_destroy(ctl_t *ctl);

#endif /* _NF_CONTROLLER_H */

