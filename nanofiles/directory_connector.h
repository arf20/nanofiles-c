#ifndef _DIRECTORY_CONNECTOR_H
#define _DIRECTORY_CONNECTOR_H

#include <sys/socket.h>

typedef struct {
    int sock; 
    const char *hostname;
    struct sockaddr addr;
} dc_t;

dc_t* dc_new(const char *hostname);
const char* dc_request(dc_t *dc, const char *rdata);
int dc_test(dc_t *dc);
int dc_ping_raw(dc_t *dc);

#endif /* _DIRECTORY_CONNECTOR_H */

