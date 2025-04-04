#include "nf_message.h"

#include "config.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char buff[MAX_NF_SEND_SIZE];

/* client requests */

size_t
nfm_filereq(const char *hash)
{

}

size_t
nfm_chunkreq(unsigned int size, size_t offset)
{

}

size_t
nfm_stop()
{

}

/* server responses */

size_t
nfm_accepted()
{

}

size_t
nfm_badfilereq()
{

}

size_t
nfm_chunk()
{

}

size_t
nfm_badchunkreq()
{

}

/* deserialize */

nf_message_t*
nfm_deserialize(const char *streamdata, size_t size)
{

}

void
nfm_destroy(nf_message_t *nfm)
{

}

