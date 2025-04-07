#ifndef _NF_CONFIG_H
#define _NF_CONFIG_H

#define DEFAULT_SHARED_DIRNAME      "nf-shared/"
#define DEFAULT_DIRECTORY_HOSTNAME  "localhost"

#define HASH_READ_SIZE              4096

#define NF_DEFAULT_P2P_PORT         10000
#define MAX_NF_BUFF_SIZE            2097165 /* 2MiB + header */
#define CHUNK_SIZE                  2097152 /* 2MiB */
#define POLL_TIMEOUT                1000    /* milliseconds */

extern int debug;

#endif /* _NF_CONFIG_H */

