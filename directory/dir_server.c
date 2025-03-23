#include "dir_server.h"

#include "../common/config.h"
#include "../common/util.h"
#include "../common/dir_message.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

dir_server_t*
ds_new(float corruption_prob)
{
    dir_server_t *ds = malloc(sizeof(dir_server_t));
    ds->corruption_prob = corruption_prob;

    ds->db = filedb_new();
    NF_TRY(
        (ds->sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == 0,
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
        .sin6_port = htons(NF_DIRECTORY_PORT),
        .sin6_addr = IN6ADDR_ANY_INIT,
        .sin6_flowinfo = 0,
        .sin6_scope_id = 0
    };

    NF_TRY(
        bind(ds->sock, (struct sockaddr*)&listen_sa, sizeof(struct sockaddr_in6))
            < 0,
        "bind", strerror(errno), return NULL
    );

    return ds;
}

const char*
ds_recv(dir_server_t *ds, struct sockaddr_in6 *client_sa)
{
    static char recv_buff[MAX_DGRAM_SIZE];

    socklen_t addrlen;

    ssize_t recvbytes = 0;

    NF_TRY(
        (recvbytes = recvfrom(ds->sock, recv_buff, MAX_DGRAM_SIZE, 0,
            (struct sockaddr*)client_sa, &addrlen)) < 0,
        "recvfrom", strerror(errno), return NULL
    );

    recv_buff[addrlen] = '\0';

    return recv_buff;
}

void
ds_respond(dir_server_t *ds, const char *recv_datagram,
    const struct sockaddr_in6 *client_sa)
{
    char addr_str_buff[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &client_sa->sin6_addr, addr_str_buff, INET6_ADDRSTRLEN);
    printf("received from %s: ", addr_str_buff);

    if (strncmp(recv_datagram, "ping", 4) == 0) {
        printf("ping -> pingok\n");
        
        NF_TRY(
            sendto(ds->sock, "pingok", 6, 0,
                (struct sockaddr*)client_sa, sizeof(struct sockaddr_in6)) < 0,
            "sendto", strerror(errno), return
        );

        return;
    } 

    
}

void
ds_run(dir_server_t *ds)
{
    struct sockaddr_in6 client_sa;
    while (1) {
        const char *recv_datagram = ds_recv(ds, &client_sa);
        ds_respond(ds, recv_datagram, &client_sa);
    }
}

