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
} nf_client_t;

nfs_t* nfs_new(unsigned short port);
int nfs_test(nfs_t *nfs);
nf_client_t* nfs_accept(nfs_t *nfs);
void nfs_destroy(nfs_t *nfs);

nf_client_t* nf_client_new(int sock, struct sockaddr_in6 addr);
void nf_client_destroy(nf_client_t *client);

#endif /* _NF_SERVER_H */

