#ifndef _DIR_MESSAGE_H
#define _DIR_MESSAGE_H

#include "filedb.h"

/* client requests */
const char* dm_ping(const char *protoid);
const char* dm_filelist();
const char* dm_publish(db_t *db);

/* direcory responses */
const char* dm_pingok();
const char* dm_filelistres(db_t *db);
const char* dm_publishack();

#endif /* _DIR_MESSAGE_H */

