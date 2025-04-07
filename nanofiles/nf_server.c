#include "nf_server.h"

#include "../common/config.h"
#include "../common/netutil.h"
#include "../common/util.h"

#include "nf_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/ip.h>
#include <arpa/inet.h>

nfs_t *
nfs_new(unsigned short port)
{ 
    nfs_t *nfs = malloc(sizeof(nfs_t));
    
    struct sockaddr_in6 listen_sa = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(port),
        .sin6_addr = IN6ADDR_ANY_INIT,
        .sin6_flowinfo = 0,
        .sin6_scope_id = 0
    };

    /* create socket */
    NF_TRY(
        (nfs->accept_sock = socket(listen_sa.sin6_family,
            SOCK_STREAM, IPPROTO_TCP)) == 0,
        "socket", strerror(errno), return NULL
    );

    /* default on linux, not on BSD */
    int no = 0;
    NF_TRY(
        setsockopt(nfs->accept_sock, IPPROTO_IPV6, IPV6_V6ONLY,
            (void*)&no, sizeof(int)) < 0,
        "setsockopt", strerror(errno), return NULL
    );

    NF_TRY(
        bind(nfs->accept_sock, (struct sockaddr*)&listen_sa,
            sizeof(struct sockaddr_in6)) < 0,
        "bind", strerror(errno), return NULL
    );

    NF_TRY(
        listen(nfs->accept_sock, SOMAXCONN) < 0,
        "listen", strerror(errno), return NULL
    );

    return nfs;
}

int
nfs_test(nfs_t *nfs)
{
    /* test: accept, recv and resend */
    int client_sock = 0;
    struct sockaddr_in6 client_addr;
    socklen_t addrlen;

    NF_TRY(
        (client_sock = accept(nfs->accept_sock,
                (struct sockaddr*)&client_addr, &addrlen)) < 0,
        "accept", strerror(errno), return 0
    );

    static char test_recv_buff[256];
    size_t recv_bytes;
    NF_TRY(
        (recv_bytes = recv(client_sock, test_recv_buff, 256, 0)) < 0,
        "recv", strerror(errno), return 0
    );

    NF_TRY(
        send(client_sock, test_recv_buff, recv_bytes, 0) < 0,
        "send", strerror(errno), return 0
    );

    NF_TRY(
        close(client_sock) < 0,
        "close", strerror(errno), return 0
    );

    return 1;
}

nf_client_t*
nfs_accept(nfs_t *nfs)
{
    int client_sock = 0;
    struct sockaddr_in6 client_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in6);

    NF_TRY(
        (client_sock = accept(nfs->accept_sock,
                (struct sockaddr*)&client_addr, &addrlen)) < 0,
        "accept", strerror(errno), return NULL
    );

    return nfs_nfc_new(client_sock, client_addr);
}


void
nfs_destroy(nfs_t *nfs)
{
    NF_TRY(
        close(nfs->accept_sock) < 0,
        "close", strerror(errno),
    );
    free(nfs);
}

nf_client_t*
nfs_nfc_new(int sock, struct sockaddr_in6 addr)
{
    nf_client_t *client = malloc(sizeof(nf_client_t));
    client->sock = sock;
    client->addr = addr;

    char addr_str_buff[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &addr.sin6_addr, addr_str_buff, INET6_ADDRSTRLEN);
    client->hostname = strdup(addr_str_buff);

    return client;
}

ssize_t
nfs_nfc_recv(const nf_client_t *client, const char **recv_data)
{
    static char recv_buff[MAX_NF_BUFF_SIZE];
    ssize_t res = 0;

    NF_TRY(
        (res = recv(client->sock, recv_buff, MAX_NF_BUFF_SIZE, 0)) < 0,
        "recv", strerror(errno), return res
    );

    *recv_data = recv_buff;
    return res;
}

int
nfs_nfc_send(const nf_client_t *client, const char *send_data, size_t size)
{
    int res = 0;
    NF_TRY(
        (res = send(client->sock, send_data, size, 0)) < 0,
        "recv", strerror(errno), return 0
    );
    return 1;
}

void
nfs_nfc_destroy(nf_client_t *client)
{
    close(client->sock);
    free(client->hostname);
    free(client);
}

