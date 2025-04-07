#include "nf_ctl_logic_p2p.h"

#include "../common/config.h"
#include "../common/util.h"
#include "../common/filedb.h"

#include "nf_message.h"
#include "nf_config.h"
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
logicp2p_new(const char *shared_dir, const filedb_t *localdb)
{
    logicp2p_t *lp = malloc(sizeof(logicp2p_t));
    lp->shared_dir = shared_dir;
    lp->localdb = localdb;

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
logicp2p_download(const logicp2p_t *lp, const file_info_t *fi, FILE *output)
{
    /* connect to all serverlist */
    nfc_t **connections = malloc(sizeof(nfc_t*) * fi->serverlist->size);
    int peers_reached = 0;
    for (int i = 0; i < fi->serverlist->size; i++) {
        printf("nf_connector: connecting to %s... ", fi->serverlist->vec[i]);
        connections[i] = nfc_new(fi->serverlist->vec[i]);
        if (connections[i]) {
            printf("ok\n");
            peers_reached++;
        } else
            printf("failed - ignored\n");
    }
       

    /* divide file in chunks */
    int chunks = (fi->size / CHUNK_SIZE) + (fi->size % CHUNK_SIZE > 0);
    chunk_state_t *chunk_states = malloc(sizeof(chunk_state_t) * chunks);
    for (int i = 0; i < chunks; i++)
        chunk_states[i] = CHUNK_QUEUED;
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
        printf("nf_connector: -> filereq\n");
    }

    if (peers_reached == 0) {
        printf("nf_connector: no peers could be reached for file %s - stopped\n", fi->name);
        return 0;
    } else
        printf("nf_connector: downloading %d chunks from %d peers...\n",
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
    int pollres = 0, stop = 0;
    while (!stop && (pollres = poll(fds, peers_reached, POLL_TIMEOUT)) >= 0)
    {
        NF_TRY(pollres == 0, "poll", "timed out, retrying...", continue);
        /* walk sockets looking for data */
        for (int i = 0; i < peers_reached; i++) {
            if (fds[i].revents == 0)
                continue;
            NF_TRY(
                fds[i].revents != POLLIN,
                "poll(.revents)", "idk, closing",
                if (connections[fd_nfc_idx[i]])
                    nfc_destroy(connections[fd_nfc_idx[i]]);
                continue;
            );

            /* receive data */
            const char *buff = NULL;
            ssize_t recv_bytes = nfc_recv(connections[fd_nfc_idx[i]], &buff);
            if (recv_bytes <= 0) {
                if (recv_bytes == 0)
                    printf("nf_connector: peer closed connection\n");
                nfc_destroy(connections[fd_nfc_idx[i]]);
                connections[fd_nfc_idx[i]] = NULL;
                continue;
            }

            switch (((const nf_header_base_t*)buff)->opcode) {
                case OP_ACCEPTED: {
                    printf("nf_connector: <- accepted\n");
                    /* request chunk */
                    int res = select_request_chunk(chunk_states, chunks,
                        connections[fd_nfc_idx[i]], fi->size);
                    if (res == 0)
                        stop = 2;
                    printf("nf_connector: -> chunkreq\n");
                } break;
                case OP_BADFILEREQ: {
                    printf("nf_connector: peer %s does not have this file\n",
                        connections[fd_nfc_idx[i]]->hostname);
                    nfc_destroy(connections[fd_nfc_idx[i]]);
                    stop = 2;
                } break;
                case OP_CHUNK: {
                    printf("nf_connector: <- chunk\n");
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

                    if (chunks_remaining == 0) {
                        stop = 1;
                        break;
                    }

                    /* and request another */
                    int res = select_request_chunk(chunk_states, chunks,
                        connections[fd_nfc_idx[i]], fi->size);
                    if (res == 0) {
                        stop = 1;
                        break;
                    }
                    printf("nf_connector: -> chunkreq\n");
                } break;
                case OP_BADCHUNKREQ: {
                    /* bad chunk request? */
                    NF_TRY(1, "nfc_receive_response", "bad chunk request", );
                    /* request another? */
                    int res = select_request_chunk(chunk_states, chunks,
                        connections[fd_nfc_idx[i]], fi->size);
                    if (res == 0)
                        stop = 1;
                    printf("nf_connector: -> chunkreq\n");
                }
                default: {
                    printf("nf_client: unknown response\n");
                } break;
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

/* data type passed to recv loop */
typedef struct {
    const filedb_t *localdb;
    const char *shared_dir;
    nf_client_t *client;
} client_loop_data_t;

void
send_badchunkreq(const nf_client_t *client)
{
    const char *send_buff = NULL;
    size_t send_size = nfm_badchunkreq(&send_buff);
    nfs_nfc_send(client, send_buff, send_size);
    printf("nf_server(%s): filereq -> badfilereq\n",
        client->hostname);
}

/* recv loop */
static void*
client_loop(void *arg)
{
    client_loop_data_t *tdata = (client_loop_data_t*)arg;

    FILE *reqd_file = NULL;

    /* recv data */
    const char *buff = NULL;
    ssize_t recv_bytes = 0;
    while ((recv_bytes = nfs_nfc_recv(tdata->client, &buff)) >= 0) {
        if (recv_bytes == 0) {
            printf("nf_server(%s): peer closed connection\n",
                tdata->client->hostname);
            break;
        }

        switch (((const nf_header_base_t*)buff)->opcode) {
            case OP_FILEREQ: {
                const nf_header_filereq_t *fr =
                    (const nf_header_filereq_t*)buff;

                char hashstr[41];
                sha1_bin2str(fr->hash, hashstr);
                
                const file_info_t *fi =
                    filedb_find_hash(tdata->localdb, hashstr);
                if (!fi) {
                    /* not found - send filereqbad */
                    const char *send_buff = NULL;
                    size_t send_size = nfm_badfilereq(&send_buff);
                    nfs_nfc_send(tdata->client, send_buff, send_size);
                    printf("nf_server(%s): filereq -> badfilereq\n",
                        tdata->client->hostname);
                    break;
                }
                
                char path[4096];
                snprintf(path, 4096, "%s%s", tdata->shared_dir, fi->name);
                
                reqd_file = fopen(path, "rb");
                NF_TRY_C(!reqd_file, "fopen", strerror(errno), path, break);
                
                printf("nf_server(%s): opened %s for reading\n",
                    tdata->client->hostname, path);

                const char *send_buff = NULL;
                size_t send_size = nfm_accepted(&send_buff);
                nfs_nfc_send(tdata->client, send_buff, send_size);
                printf("nf_server(%s): filereq -> accept\n",
                    tdata->client->hostname);
            } break;
            case OP_CHUNKREQ: {
                if (!reqd_file)
                    send_badchunkreq(tdata->client);

                const nf_header_chunk_t *cr =
                    (const nf_header_chunk_t*)buff;

                char *send_buff = NULL;
                size_t send_size = nfm_chunk(&send_buff, cr->size, cr->offset);
                char *send_chunk_data = send_buff + send_size;

                NF_TRY(
                    fseek(reqd_file, cr->offset, SEEK_SET) < 0,
                    "fseek", strerror(errno), send_badchunkreq(tdata->client);
                    break
                );

                size_t read_bytes = 0;
                NF_TRY(
                    (read_bytes = fread(send_chunk_data, 1, cr->size,
                        reqd_file)) < 0,
                    "fread", strerror(errno), send_badchunkreq(tdata->client);
                    break
                );

                if (read_bytes != cr->size) {
                    send_badchunkreq(tdata->client);
                    break;
                }

                nfs_nfc_send(tdata->client, send_buff, send_size + read_bytes);
                printf("nf_server(%s): chunkreq -> chunk\n",
                    tdata->client->hostname);
            } break;
            case OP_STOP: {
                /* unimplemented */
            } break;
            default: {
                printf("nf_server: unknown request\n");
            } break;
        }
    }

    nfs_nfc_destroy(tdata->client);
    free(tdata);

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
            client_loop_data_t *tdata = malloc(sizeof(client_loop_data_t));
            tdata->shared_dir = lp->shared_dir;
            tdata->localdb = lp->localdb;
            tdata->client = client;

            pthread_t client_thread;
            pthread_create(&client_thread, NULL, client_loop, tdata);
            pthread_detach(client_thread);

            inet_ntop(AF_INET6, &client->addr.sin6_addr, addr_str_buff,
                INET6_ADDRSTRLEN);
            printf("nf_server: accepted client %s\n", addr_str_buff);
        }
    } while (client);

    nfs_destroy(lp->nfs);
    
    return NULL;
}

void
logicp2p_start_server(logicp2p_t *lp)
{
    lp->nfs = nfs_new(NF_DEFAULT_P2P_PORT);
    if (!lp->nfs) {
        printf("failed starting server\n");
        return;
    }

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

