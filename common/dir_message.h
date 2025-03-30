#ifndef _DIR_MESSAGE_H
#define _DIR_MESSAGE_H

#include "filedb.h"

/* serialize directory messages from arguments */
/* client requests */
const char* dm_ping(const char *protoid);
const char* dm_filelist();
const char* dm_publish(filedb_t *db);

/* direcory responses */
const char* dm_pingok();
const char* dm_pingbad();
const char* dm_filelistres(filedb_t *db);
const char* dm_publishack();

/* deserialize directory message */
typedef enum {
    OPER_INVALID,
    OPER_PING,
    OPER_FILELIST,
    OPER_PUBLISH,
    OPER_PINGOK,
    OPER_PINGBAD,
    OPER_FILELISTRES,
    OPER_PUBLISHACK
} operation_t;

typedef struct {
    operation_t operation;
    char *data;
    size_t size;
} dir_message_t;

typedef struct {
    char *protocolid;
} dir_message_ping_t;

typedef struct {
    filedb_t *filelist;
    unsigned short port;
} dir_message_publish_t;

typedef struct {
    filedb_t *db;
} dir_message_filelistres_t;

dir_message_t *dm_deserialize(const char *datagram);
void dm_deserialize_ping(dir_message_t *dm, const char *datagram);
void dm_deserialize_publish(dir_message_t *dm, const char *datagram);
void dm_deserialize_filelistres(dir_message_t *dm, const char *datagram);
void dm_destroy(dir_message_t *dm);

#endif /* _DIR_MESSAGE_H */

