#include "dir_message.h"

#include "config.h"

#include <stdio.h>
#include <string.h>

static char buff[MAX_DGRAM_SIZE];

/* client requests */

const char*
dm_ping(const char *protoid)
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: ping\nprotocol: %s\n",
        NF_PROTOCOL_ID);
    return buff;
}

const char*
dm_filelist()
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: filelist\n");
    return buff;
}

const char*
dm_publish(db_t *db)
{
    int len = 0:
    len += snprintf(buff, MAX_DGRAM_SIZE, "operation: publish\n");
   
    for (int i = 0; i < db->size; i++) {
        len += snprintf(buff + len, MAX_DGRAM_SIZE - len,
            "%s: %d: %s\n",
            db->vec[i].hash, db->vec[i].size, db->vec[i].filename);
    }
     
    return buff;
}

/* direcory responses */

const char*
dm_pingok()
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: pingok\n");
    return buff;
}

const char*
dm_filelistres(db_t *db)
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: filelistres\n");
    /* TODO */
    return buff;
}

const char*
dm_publishack()
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: publishack\n");
    return buff;
}

