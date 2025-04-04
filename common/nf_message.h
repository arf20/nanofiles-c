#ifndef _NF_MESSAGE_H
#define _NF_MESSAGE_H

#include "filedb.h"

/* serialize directory messages from arguments */
/* client requests */

size_t nfm_filereq(const char *hash);
size_t nfm_chunkreq(unsigned int size, size_t offset);
size_t nfm_stop();

/* direcory responses */

size_t nfm_accepted();
size_t nfm_badfilereq();
size_t nfm_chunk();
size_t nfm_badchunkreq();

/* deserialize directory message */
typedef enum {
    OP_FILEREQ,
    OP_CHUNKREQ,
    OP_STOP,
    OP_ACCEPTED,
    OP_BADFILEREQ,
    OP_CHUNK,
    OP_BADCHUNKREQ
} opcode_t;

typedef struct {
    opcode_t operation;
    char *data;
    size_t size;
} nf_message_t;

/* deserialize */

nf_message_t *nfm_deserialize(const char *streamdata, size_t size);
void nfm_destroy(nf_message_t *nfm);

#endif /* _NF_MESSAGE_H */

