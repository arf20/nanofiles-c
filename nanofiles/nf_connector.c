#include "nf_connector.h"

#include "../common/config.h"
#include "../common/netutil.h"
#include "../common/util.h"

#include "nf_message.h"
#include "nf_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/ip.h>

nfc_t *
nfc_new(const char *hostname)
{ 
    nfc_t *nfc = malloc(sizeof(nfc_t));
    nfc->hostname = strdup(hostname);
    nfc->addr = malloc(sizeof(struct sockaddr_storage));
    
    /* resolve address */
    if (!resolve_name(hostname, nfc->addr))
        return NULL; /* error prop */

    /* set port, sin_port is the same field in both in and in6 */
    ((struct sockaddr_in*)nfc->addr)->sin_port = htons(NF_DEFAULT_P2P_PORT);

    /* create socket */
    NF_TRY(
        (nfc->sock = socket(nfc->addr->sa_family, SOCK_STREAM, IPPROTO_TCP)) == 0,
        "socket", strerror(errno), return NULL
    );

    /* if IPv4 mapped as v6 */
    if (nfc->addr->sa_family == AF_INET6) {
        int no = 0;
        NF_TRY(
            setsockopt(nfc->sock, IPPROTO_IPV6, IPV6_V6ONLY,
                (void*)&no, sizeof(int)) < 0,
            "setsockopt", strerror(errno), return NULL
        );
    }

    /* connect to server */
    NF_TRY(
        connect(nfc->sock, nfc->addr, sa_len(nfc->addr)),
        "connect", strerror(errno), return NULL
    )

    return nfc;
}

int
nfc_test(const nfc_t *nfc)
{
    /* test: send int, recv and check */
    static const int test_int = 69420;
    NF_TRY(
        send(nfc->sock, &test_int, sizeof(test_int), 0) < 0,
        "send", strerror(errno), return 0
    )

    static char test_recv_buff[256];
    NF_TRY(
        recv(nfc->sock, test_recv_buff, 256, 0) < 0,
        "recv", strerror(errno), return 0
    )

    return *(int*)test_recv_buff == test_int;
}

ssize_t
nfc_recv(const nfc_t *nfc, const char **buff)
{
    char recv_buff[MAX_NF_BUFF_SIZE];
    ssize_t res = 0;

    NF_TRY(
        (res = recv(nfc->sock, recv_buff, MAX_NF_BUFF_SIZE, 0)) < 0,
        "recv", strerror(errno), return res
    )

    *buff = recv_buff;
    return res;
}

int
nfc_request_file(const nfc_t *nfc, const char *hash)
{
    const char *buff = NULL;
    size_t reqsize = nfm_filereq(&buff, hash);
    
    NF_TRY(
        send(nfc->sock, buff, reqsize, 0) < 0,
        "send", strerror(errno), return 0
    )

    return 1;
}

int
nfc_request_chunk(const nfc_t *nfc, size_t offset, unsigned int size)
{
    const char *buff = NULL;
    size_t reqsize = nfm_chunkreq(&buff, size, offset);
    
    NF_TRY(
        send(nfc->sock, buff, reqsize, 0) < 0,
        "send", strerror(errno), return 0
    )

    return 1;
}

int
nfc_stop(const nfc_t *nfc)
{
    const char *buff = NULL;
    size_t reqsize = nfm_stop(&buff);
    
    NF_TRY(
        send(nfc->sock, buff, reqsize, 0) < 0,
        "send", strerror(errno), return 0
    )

    return 1;
}

void
nfc_destroy(nfc_t *nfc)
{
    close(nfc->sock);
    free(nfc->hostname);
    free(nfc);
}

