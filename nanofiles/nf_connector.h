#ifndef _NF_CONNECTOR_H
#define _NF_CONNECTOR_H

#include <sys/socket.h>

typedef struct {
    int sock; 
    struct sockaddr addr;
    char *hostname;
} nfc_t;

nfc_t* nfc_new(const char *hostname);
int nfc_test(const nfc_t *nfc);
int nfc_receive_response(const nfc_t *nfc, const char **buff);
int nfc_request_file(const nfc_t *nfc, const char *hash);
int nfc_request_chunk(const nfc_t *nfc, size_t offset, unsigned int size);
void nfc_destroy(nfc_t *nfc);

#endif /* _NF_CONNECTOR_H */

