#ifndef _NF_MESSAGE_H
#define _NF_MESSAGE_H

#include "../common/filedb.h"

#include <stdint.h>

/* serialize directory messages from arguments */
/* client requests */

size_t nfm_filereq(const char **buff, const char *hash);
size_t nfm_chunkreq(const char **buff, unsigned int size, size_t offset);
size_t nfm_stop(const char **buff);

/* peer responses */

size_t nfm_accepted(const char **buff);
size_t nfm_badfilereq(const char **buff);
size_t nfm_chunk(char **buff, unsigned int size, size_t offset);
size_t nfm_badchunkreq(const char **buff);

/* deserialize directory message */
typedef enum {
    OP_FILEREQ = 0x01,
    OP_CHUNKREQ,
    OP_STOP,
    OP_ACCEPTED = 0x11,
    OP_BADFILEREQ,
    OP_CHUNK,
    OP_BADCHUNKREQ
} opcode_t;

/* we are sending this thorugh a socket, so pack it with no gaps */
typedef struct __attribute__((packed)) {
    uint8_t opcode;
} nf_header_base_t;

typedef struct __attribute__((packed)) {
    uint8_t opcode;
    uint8_t hash[20]; 
} nf_header_filereq_t;

typedef struct __attribute__((packed)) {
    uint8_t opcode;
    uint64_t offset;
    uint32_t size;
} nf_header_chunk_t;

#endif /* _NF_MESSAGE_H */

