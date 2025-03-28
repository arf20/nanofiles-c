#ifndef _NF_CONTROLLER_H
#define _NF_CONTROLLER_H

#include "directory_connector.h"
#include "nf_shell.h"

typedef enum { OFFLINE, ONLINE } status_t;

typedef struct {
    const char *directory_hostname;
    status_t state;
    dc_t *dc;
    shell_t *shell;
    int quit;
} ctl_t;

ctl_t* ctl_new(const char *directory_hostname);
void ctl_test_directory(ctl_t *ctl);
void ctl_process_command(ctl_t *ctl);
int ctl_should_quit(ctl_t *ctl);
void ctl_destroy(ctl_t *ctl);

#endif /* _NF_CONTROLLER_H */

