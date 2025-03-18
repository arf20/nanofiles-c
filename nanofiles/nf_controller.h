#ifndef _NF_CONTROLLER_H
#define _NF_CONTROLLER_H

typedef enum { OFFLINE, ONLINE } status_t;

typedef struct {
    const char *directory_hostname;
    status_t state;
    dc_t *dc;
} ctl_t;

ctl_t* ctl_new(const char *shared_dir);

#endif /* _NF_CONTROLLER_H */

