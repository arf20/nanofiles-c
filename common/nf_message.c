#include "nf_message.h"

#include "config.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char msg_buff[MAX_NF_BUFF_SIZE];

/* client requests */

size_t
nfm_filereq(char **buff, const char *hash)
{
    nf_header_filereq_t *header = (nf_header_filereq_t*)msg_buff;
    header->opcode = OP_FILEREQ;
    sha1_str2bin(hash, header->hash);

    *buff = msg_buff;
    return sizeof(nf_header_filereq_t);
}

size_t
nfm_chunkreq(char **buff, unsigned int size, size_t offset)
{
    nf_header_chunk_t *header = (nf_header_chunk_t*)msg_buff;
    header->opcode = OP_CHUNKREQ;
    header->offset = offset;
    header->size = size;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

size_t
nfm_stop(char **buff)
{
    nf_header_base_t *header = (nf_header_base_t*)msg_buff;
    header->opcode = OP_STOP;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

/* server responses */

size_t
nfm_accepted(char **buff)
{
    nf_header_base_t *header = (nf_header_base_t*)msg_buff;
    header->opcode = OP_ACCEPTED;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

size_t
nfm_badfilereq(char **buff)
{
    nf_header_base_t *header = (nf_header_base_t*)msg_buff;
    header->opcode = OP_BADFILEREQ;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

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
nfm_badchunkreq(char **buff)
{
    nf_header_base_t *header = (nf_header_base_t*)msg_buff;
    header->opcode = OP_BADCHUNKREQ;

    *buff = msg_buff;
    return sizeof(nf_header_chunk_t);
}

