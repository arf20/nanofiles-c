#include "nf_ctl_logic_p2p.h"

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

logicp2p_t*
logicp2p_new()
{
    logicp2p_t *lp = malloc(sizeof(logicp2p_t));

    lp->nfs = nfs_new(directory_hostname);

    return ld;
}

int
logicp2p_test_client(logicp2p_t *lp, const char *hostname)
{
    printf("testing connection to peer %s... ", hostname);
    fflush(stdout);

    nfc_t *nfc = nfc_new(hostname);

    if (nfc_test(nfc)) {
        printf("online\n");
        nfc_destroy(nfc);
        return 1;
    } else {
        printf("offline\n");
        nfc_destroy(nfc);
        return 0;
    }
}

void
logicp2p_test_server(logicp2p_t *lp)
{
    printf("testing connection from peer %s... ", hostname);
    fflush(stdout);

    nfs_test(lp->nfs);
}

void
logicp2p_start_server(logicp2p_t *lp)
{

}

void
logicp2p_destroy(logicp2p_t *lp)
{
    dc_destroy(ld->dc);
    free(lp);
}

