#ifndef _NF_CONNECTOR_H
#define _NF_CONNECTOR_H

#include <sys/socket.h>

typedef struct {
    int sock; 
    struct sockaddr addr;
} nfc_t;

nfc_t* nfc_new(const char *hostname);
int nfc_test(nfc_t *nfc);
void nfc_destroy(nfc_t *nfc);

#endif /* _NF_CONNECTOR_H */

