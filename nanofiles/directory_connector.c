#include "directory_connector.h"

#include "config.h"

#include "netutil.h"
#include "util.h"
#include "dir_message.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <netinet/ip.h>

dc_t *
dc_new(const char *hostname)
{ 
    dc_t *dc = malloc(sizeof(dc_t));
    
    dc->hostname = hostname;

    /* resolve address */
    dc->addr = resolve_name(hostname);
    if (dc->addr.sa_family == 0)
        return NULL; /* error prop */

    /* create socket */
    NF_TRY(
        (dc->sock = socket(dc->addr.sa_family, SOCK_DGRAM, IPPROTO_UDP)) == 0,
        "socket", strerror(errno), return NULL
    );

    /* set socket receive timeout option now */
    if (sock_set_timeout(dc->sock, SOCK_TIMEOUT) < 0)
        return NULL; /* error prop */

    return dc;
}


const char *
dc_request(dc_t *dc, const char *rdata)
{
    static char recv_buff[MAX_DGRAM_SIZE];
    struct sockaddr src_addr = { 0 };
    socklen_t addrlen = 0;
    int fwds = 0;

    NF_TRY(
        !dc->addr.sa_family || !dc->sock,
        "dc_request", "address or socket invalid\n", return NULL
    );

    for (; fwds < MAX_FORWARDINGS; fwds++) {
        NF_TRY(
            sendto(dc->sock, rdata, strlen(rdata), 0, &dc->addr,
                sa_len(&dc->addr)) < 0,
            "sendto", strerror(errno), return NULL
        );

        NF_TRY(
            recvfrom(dc->sock, recv_buff, MAX_DGRAM_SIZE, 0, &src_addr,
                &addrlen) < 0,
            "recvfrom", strerror(errno), return NULL
        );
    }

    NF_TRY(
        fwds >= MAX_FORWARDINGS,
        "dc_request",
        "maximum number of forwardings reached, directory unreachable\n",
        return NULL
    );

    return recv_buff;
}

int
dc_test(dc_t *dc)
{
    const char *res = dc_request(dc, "ping");
    if (!res)
        return -1;

    return strncmp(res, "pingok", 6) == 0;
}

int
dc_ping_raw(dc_t *dc)
{
    const char *message_ping = dm_ping(NF_PROTOCOL_ID);

    const char *res = dc_request(dc, message_ping);
    if (!res)
        return -1;

    return strncmp(res, "welcome", 6) == 0;
}

