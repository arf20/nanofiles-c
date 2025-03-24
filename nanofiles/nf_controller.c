#include "nf_controller.h"

#include "nf_shell.h"

#include <stdlib.h>
#include <stdio.h>

ctl_t*
ctl_new(const char *directory_hostname)
{
    ctl_t *ctl = malloc(sizeof(ctl_t));

    ctl->state = OFFLINE;
    ctl->dc = dc_new(directory_hostname);
    ctl->shell = shell_new();

    return ctl;
}

void
ctl_test_directory(ctl_t *ctl)
{
    printf("testing connection to directory... ");
    if (dc_test(ctl->dc)) {
        printf("online\n");
        ctl->state = ONLINE;
    } else {
        printf("offline\n");
        ctl->state = OFFLINE;
    }
}

void
ctl_process_command(ctl_t *ctl, const char *cmd)
{
    
}

void
ctl_destroy(ctl_t *ctl)
{
    shell_destroy(ctl->shell);
    dc_destroy(ctl->dc);
}

