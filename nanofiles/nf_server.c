#include "nf_server.h"

#include "../common/config.h"

#include "../common/netutil.h"
#include "../common/util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/ip.h>

nfs_t *
nfs_new(unsigned short port)
{ 
    nfs_t *nfs = malloc(sizeof(nfs_t));
    
    /* create socket */
    NF_TRY(
        (nfs->sock = socket(nfs->addr.sa_family, SOCK_STREAM, IPPROTO_TCP)) == 0,
        "socket", strerror(errno), return NULL
    );

    /* default on linux, not on BSD */
    int no = 0;
    NF_TRY(
        setsockopt(ds->sock, IPPROTO_IPV6, IPV6_V6ONLY,
            (void*)&no, sizeof(int)) < 0,
        "setsockopt", strerror(errno), return NULL
    );

    struct sockaddr_in6 listen_sa = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(NF_P2P_PORT),
        .sin6_addr = IN6ADDR_ANY_INIT,
        .sin6_flowinfo = 0,
        .sin6_scope_id = 0
    };

    NF_TRY(
        bind(ds->sock, (struct sockaddr*)&listen_sa,
            sizeof(struct sockaddr_in6)) < 0,
        "bind", strerror(errno), return NULL
    );

    NF_TRY(
        listen(ds->sock, SOMAXCONN) < 0,
        "listen", strerror(errno), return NULL
    );

    return nfs;
}

int
nfs_test(nfs_t *nfs)
{
    /* test: recv and resend */
    static char test_recv_buff[256];
    size_t recv_bytes;
    NF_TRY(
        (recv_bytes = recv(nfs->sock, test_recv_buff, 256, 0)) < 0,
        "recv", strerror(errno), return 0
    )

    NF_TRY(
        send(nfs->sock, test_recv_buff, recv_bytes, 0) < 0,
        "send", strerror(errno), return 0
    )

    return *(int*)test_recv_buff == test_int;
}

void
nfs_destroy(nfs_t *nfs)
{
    close(nfs->sock);
}

