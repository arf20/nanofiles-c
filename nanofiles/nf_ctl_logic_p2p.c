#include "nf_ctl_logic_p2p.h"

#include "../common/config.h"
#include "../common/util.h"
#include "../common/nf_message.h"
#include "nf_connector.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

#include <arpa/inet.h>
#include <pthread.h>

typedef enum {
    CHUNK_QUEUED,
    CHUNK_AWAITING,
    CHUNK_FINISHED
} chunk_state_t;

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

/* sequential download */
static int
select_next_chunk(const chunk_state_t *chunk_states, int chunks,
    size_t *offset, unsigned int *size, const size_t *filesize)
{
    int chunk = 0, found = 0;
    for (; chunk < chunks; chunk++) {
        if (chunk_states[chunk] == CHUNK_QUEUED) {
            found = 1;
            break;
        }
    }
    if (!found)
        return -1;
    if (offset)
        *offset = CHUNK_SIZE * chunk;
    if (size)
        *size = (*filesize % CHUNK_SIZE && chunk == chunks - 1) ?
            *filesize % CHUNK_SIZE : CHUNK_SIZE;
    return chunk;
}

static int
select_request_chunk(chunk_state_t *chunk_states, int chunks,
    nfc_t *nfc, size_t filesize) 
{
    size_t offset = 0;
    unsigned int size = 0;
    int chunk = -1;
    if ((chunk = select_next_chunk(chunk_states, chunks,
        &offset, &size, &filesize)) < 0)
    {
        /* finished downlad */
        return 0;
    }
    if (!nfc_request_chunk(nfc, offset, size)) {
        nfc_destroy(nfc);
        return -1;
    }

    chunk_states[chunk] = CHUNK_AWAITING;

    return 1;
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
       

    /* divide file in chunks */
    int chunks = (fi->size / CHUNK_SIZE) + (fi->size % CHUNK_SIZE);
    chunk_state_t *chunk_states = malloc(sizeof(chunk_state_t) * chunks);
    int chunks_remaining = chunks;

    /* first send: tell peer what file to download */
    for (int i = 0; i < fi->serverlist->size; i++) {
        if (!connections[i])
            continue;

        if (!nfc_request_file(connections[i], fi->hash)) {
            nfc_destroy(connections[i]);
            connections[i] = NULL;
            peers_reached--;
        }
    }

    if (peers_reached == 0) {
        printf("no peers could be reached for file %s - stopped\n", fi->name);
        return 0;
    } else
        printf("downloading %d chunks from %d peers...\n",
            chunks, peers_reached);


    /* set up fds for poll */
    struct pollfd *fds = malloc(sizeof(struct pollfd) * peers_reached);
    int *fd_nfc_idx = malloc(sizeof(struct pollfd) * peers_reached);
    for (int fdidx = 0, i = 0; i < fi->serverlist->size; i++) {
        if (!connections[i])
            continue;
        fds[fdidx].fd = connections[i]->sock;
        fds[fdidx].events = POLLIN;
        fd_nfc_idx[fdidx] = i;
        fdidx++;
    }

    /* poll loop */
    int pollres = 0, finished = 0;
    while (!finished && (pollres = poll(fds, peers_reached, POLL_TIMEOUT)) < 0) {
        NF_TRY(pollres == 0, "poll", "timed out, retrying...", );
        /* walk sockets looking for data */
        for (int i = 0; i < peers_reached; i++) {
            if (fds[i].revents == 0)
                continue;
            NF_TRY(
                fds[i].revents != POLLIN,
                "poll(.revents)", "idk, closing",
                nfc_destroy(connections[fd_nfc_idx[i]]);
                continue;
            );

            /* receive data */
            const char *buff = NULL;
            if (!nfc_receive_response(connections[fd_nfc_idx[i]], &buff)) {
                nfc_destroy(connections[fd_nfc_idx[i]]);
                continue;
            }

            switch (((const nf_header_base_t*)buff)->opcode) {
                case OP_ACCEPTED: {
                    /* request chunk */
                    int res = select_request_chunk(chunk_states, chunks,
                        connections[fd_nfc_idx[i]], fi->size);
                    if (res == 0)
                        finished = 1;
                } break;
                case OP_BADFILEREQ: {
                    printf("peer %s does not have this file\n",
                        connections[fd_nfc_idx[i]]->hostname);
                    nfc_destroy(connections[fd_nfc_idx[i]]);
                } break;
                case OP_CHUNK: {
                    /* we got a chunk, write it */
                    const nf_header_chunk_t *chunk_header =
                        (const nf_header_chunk_t*)buff;
                    NF_TRY(
                        chunk_header->offset % CHUNK_SIZE,
                        "nfc_receive_response", "we did not ask for this chunk",
                    );
                    int chunk = chunk_header->offset / CHUNK_SIZE;
                    chunk_states[chunk] = CHUNK_FINISHED;
                    fseek(output, chunk_header->offset, SEEK_SET);
                    fwrite(buff + sizeof(nf_header_chunk_t),
                        1, chunk_header->size, output);
                    chunks_remaining--;

                    /* and request another */
                    int res = select_request_chunk(chunk_states, chunks,
                        connections[fd_nfc_idx[i]], fi->size);
                    if (res == 0)
                        finished = 1;
                } break;
                case OP_BADCHUNKREQ: {
                    /* bad chunk request? */
                    NF_TRY(1, "nfc_receive_response", "bad chunk request", );
                    /* request another? */
                    int res = select_request_chunk(chunk_states, chunks,
                        connections[fd_nfc_idx[i]], fi->size);
                    if (res == 0)
                        finished = 1;
                }
            }
        }
    }

    NF_TRY(pollres < 0, "poll", strerror(errno), );


    /* disconnect clients */
    for (int i = 0; i < fi->serverlist->size; i++)
        if (connections[i])
            nfc_destroy(connections[i]);

    printf("file downloaded.\n");

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

