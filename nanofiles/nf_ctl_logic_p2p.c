#include "nf_ctl_logic_p2p.h"

#include "../common/config.h"
#include "../common/util.h"
#include "nf_connector.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <pthread.h>

logicp2p_t*
logicp2p_new()
{
    logicp2p_t *lp = malloc(sizeof(logicp2p_t));

    return lp;
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

int
logicp2p_download(const logicp2p_t *lp, const file_info_t *fi)
{
    /* open output file */
    FILE *output = fopen(fi->name, "wb");
    NF_TRY(!output, "fopen", strerror(errno), return 0);
    /* connect to all serverlist */
    nfc_t **connections = malloc(sizeof(nfc_t*) * fi->serverlist->size);
    int peers_reached = 0;
    for (int i = 0; i < fi->serverlist->size; i++) {
        printf("connecting to %s... ", fi->serverlist->vec[i]);
        connections[i] = nfc_new(fi->serverlist->vec[i]);
        if (connections[i]) {
            printf("ok\n");
            peers_reached++;
        } else
            printf("failed - ignored\n");
    }
    if (peers_reached == 0) {
        printf("no peers could be reached for file %s - stopped\n", fi->name);
        return 0;
    } else
        printf("downloading from %d peers...\n", peers_reached);
   




    /* disconnect clients */
    for (int i = 0; i < fi->serverlist->size; i++)
        if (connections[i])
            nfc_destroy(connections[i]);

    return 1;
}

int
logicp2p_test_server(logicp2p_t *lp)
{
    printf("testing server... ");
    fflush(stdout);

    lp->nfs = nfs_new(NF_DEFAULT_P2P_PORT);
    int res = nfs_test(lp->nfs);
    printf(res ? "ok\n" : "failed\n");
    nfs_destroy(lp->nfs);
    return res;
}

static void*
client_loop(void *arg)
{
    nf_client_t *client = (nf_client_t*)arg;


    nf_client_destroy(client);

    return NULL;
}

static void*
accept_loop(void *arg)
{
    logicp2p_t *lp = (logicp2p_t*)arg;
            
    char addr_str_buff[INET6_ADDRSTRLEN];
    nf_client_t *client = NULL;
    do {
        client = nfs_accept(lp->nfs);
        if (client) {
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, client_loop, client);
            pthread_detach(client_thread);

            inet_ntop(AF_INET6, &client->addr.sin6_addr, addr_str_buff,
                INET6_ADDRSTRLEN);
            printf("accepted client %s\n", addr_str_buff);
        }
    } while (client);

    nfs_destroy(lp->nfs);
    
    return NULL;
}

void
logicp2p_start_server(logicp2p_t *lp)
{
    lp->nfs = nfs_new(NF_DEFAULT_P2P_PORT);

    printf("started accept thread\n");
    pthread_t accept_thread;
    pthread_create(&accept_thread, NULL, accept_loop, lp);
    pthread_detach(accept_thread);
}

void
logicp2p_destroy(logicp2p_t *lp)
{
    free(lp);
}

