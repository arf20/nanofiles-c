#ifndef _DIRECTORY_CONNECTOR_H
#define _DIRECTORY_CONNECTOR_H

#include <sys/socket.h>

typedef struct {
    int sock; 
    const char *hostname;
    struct sockaddr addr;
} dc_t;

#endif /* _DIRECTORY_CONNECTOR_H */

