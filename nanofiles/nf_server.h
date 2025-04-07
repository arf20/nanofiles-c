#ifndef _NF_SERVER_H
#define _NF_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
    int accept_sock; 
} nfs_t;

typedef struct {
    int sock;
    struct sockaddr_in6 addr;
    char *hostname;
} nf_client_t;

nfs_t* nfs_new(unsigned short port);
int nfs_test(nfs_t *nfs);
nf_client_t* nfs_accept(nfs_t *nfs);
void nfs_destroy(nfs_t *nfs);

nf_client_t* nfs_nfc_new(int sock, struct sockaddr_in6 addr);
ssize_t nfs_nfc_recv(const nf_client_t *client, const char **recv_data);
int nfs_nfc_send(const nf_client_t *client, const char *send_data, size_t size);
void nfs_nfc_destroy(nf_client_t *client);

#endif /* _NF_SERVER_H */

