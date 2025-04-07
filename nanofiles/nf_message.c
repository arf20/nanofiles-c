#include "nf_message.h"

#include "../common/config.h"
#include "../common/util.h"
#include "nf_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char msg_buff[MAX_NF_BUFF_SIZE];

/* client requests */

size_t
nfm_filereq(const char **buff, const char *hash)
{
    nf_header_filereq_t *header = (nf_header_filereq_t*)msg_buff;
    header->opcode = OP_FILEREQ;
    sha1_str2bin(hash, header->hash);

    *buff = msg_buff;
    return sizeof(nf_header_filereq_t);
}

size_t
nfm_chunkreq(const char **buff, unsigned int size, size_t offset)
{
    nf_header_chunk_t *header = (nf_header_chunk_t*)msg_buff;
    header->opcode = OP_CHUNKREQ;
    header->offset = offset;
    header->size = size;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

size_t
nfm_stop(const char **buff)
{
    nf_header_base_t *header = (nf_header_base_t*)msg_buff;
    header->opcode = OP_STOP;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

/* server responses */

size_t
nfm_accepted(const char **buff)
{
    nf_header_base_t *header = (nf_header_base_t*)msg_buff;
    header->opcode = OP_ACCEPTED;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

size_t
nfm_badfilereq(const char **buff)
{
    nf_header_base_t *header = (nf_header_base_t*)msg_buff;
    header->opcode = OP_BADFILEREQ;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

/* only fills the header */
size_t
nfm_chunk(char **buff, unsigned int size, size_t offset)
{
    nf_header_chunk_t *header = (nf_header_chunk_t*)msg_buff;
    header->opcode = OP_CHUNKREQ;
    header->offset = offset;
    header->size = size;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

size_t
nfm_badchunkreq(const char **buff)
{
    nf_header_base_t *header = (nf_header_base_t*)msg_buff;
    header->opcode = OP_BADCHUNKREQ;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

