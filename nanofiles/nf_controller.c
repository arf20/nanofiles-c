#include "nf_controller.h"

#include <stdlib.h>
#include <stdio.h>

ctl_t*
ctl_new(const char *directory_hostname)
{
    ctl_t *ctl = malloc(sizeof(ctl_t));
    ctl->directory_hostname = directory_hostname;

    ctl->state = OFFLINE;
    ctl->dc = dc_new(directory_hostname);

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

