#ifndef _CONFIG_H
#define _CONFIG_H

#define INITIAL_VECTOR_CAPACITY     64
#define CMD_BUFF_SIZE               256

/* directory protocol */
#define NF_DIRECTORY_PORT           6868
#define NF_DIR_PROTOCOL_ID          "20032005"
#define MAX_DGRAM_SIZE              65507   /* UDP packet */
#define MAX_FORWARDINGS             5
#define SOCK_TIMEOUT                1       /* seconds */
/* nf protocol */
#define NF_DEFAULT_P2P_PORT         10000
#define MAX_NF_SEND_SIZE            2097165 /* 2MiB + header */

#endif /* _CONFIG_H */

