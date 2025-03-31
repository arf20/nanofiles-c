#include "nf_connector.h"

#include "../common/config.h"

#include "../common/netutil.h"
#include "../common/util.h"

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
    
    /* resolve address */
    nfc->addr = resolve_name(hostname);
    if (nfc->addr.sa_family == 0)
        return NULL; /* error prop */

    /* set port, sin_addr is the same field in both in and in6 */
    ((struct sockaddr_in*)&nfc->addr)->sin_port = htons(NF_P2P_PORT);

    /* create socket */
    NF_TRY(
        (nfc->sock = socket(nfc->addr.sa_family, SOCK_STREAM, IPPROTO_TCP)) == 0,
        "socket", strerror(errno), return NULL
    );

    /* connect to server */
    NF_TRY(
        connect(nfc->sock, &nfc->addr, sizeof(nfc->addr)),
        "connect", strerror(errno), return NULL
    )

    return nfc;
}

int
nfc_test(nfc_t *nfc)
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

void
nfc_destroy(nfc_t *nfc)
{
    close(nfc->sock);
}

