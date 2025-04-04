#include "dir_message.h"

#include "config.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char buff[MAX_DGRAM_SIZE];

/* client requests */

const char*
dm_ping(const char *protoid)
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: ping\nprotocol: %s\n",
        NF_DIR_PROTOCOL_ID);
    return buff;
}

const char*
dm_filelist()
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: filelist\n");
    return buff;
}

const char*
dm_publish(const filedb_t *db)
{
    int len = 0;
    len += snprintf(buff, MAX_DGRAM_SIZE, "operation: publish\n");
   
    for (int i = 0; i < db->size; i++) {
        len += snprintf(buff + len, MAX_DGRAM_SIZE - len,
            "%s: %s; %ld\n",
            db->vec[i].hash, db->vec[i].name, db->vec[i].size);
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
dm_pingbad()
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: pingbad\n");
    return buff;
}

const char*
dm_filelistres(const filedb_t *db)
{
    size_t off =
        snprintf(buff, MAX_DGRAM_SIZE, "operation: filelistres\n");
    
    for (size_t i = 0; i < db->size; i++) {
        off += snprintf(buff + off, MAX_DGRAM_SIZE - off,
            "%s: %s; %ld; ", db->vec[i].hash, db->vec[i].name, db->vec[i].size);

        for (size_t j = 0; j < db->vec[i].serverlist->size; j++) {
            off += snprintf(buff + off, MAX_DGRAM_SIZE - off,
                "%s", db->vec[i].serverlist->vec[j]);

            if (j != db->vec[i].serverlist->size - 1)
                off += snprintf(buff + off, MAX_DGRAM_SIZE - off, "; ");
        }
        off += snprintf(buff + off, MAX_DGRAM_SIZE - off, "\n");
    }


    return buff;
}

const char*
dm_publishack()
{
    snprintf(buff, MAX_DGRAM_SIZE, "operation: publishack\n");
    return buff;
}

/* deserialization */

const char*
strip(const char *s) {
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

const char*
get_value(const char *key, const char *datagram)
{
    static char buff[256];

    while (datagram && *datagram) {
        if (*datagram == '\n')
            datagram++;
        datagram = strip(datagram);
        const char *keyend = strpbrk(datagram, ":\n");

        if (strncmp(datagram, key, keyend - datagram) != 0) {
            datagram = strchr(datagram, '\n');
            continue;
        }

        if (*keyend == ':') {
            datagram = strip(keyend + 1);
            const char *valueend = strchr(datagram, '\n');
            if (!valueend)
                valueend = datagram + strlen(datagram);
            int len = MIN(valueend - datagram, 255);
            strncpy(buff, datagram, len);
            buff[len] = '\0';
            return buff;
        } else {
            buff[0] = '\0';
            return buff;
        }
    }

    return NULL;
}

dir_message_t*
dm_deserialize(const char *datagram)
{
    const char *opstr = NULL;

    NF_TRY_C(
        !(opstr = get_value("operation", datagram)),
        "get_value", "key not found", "operation", return NULL
    );

    dir_message_t *dm = malloc(sizeof(dir_message_t));
    dm->data = NULL;
    dm->size = 0;

    if (strcmp(opstr, "ping") == 0)
        dm->operation = OPER_PING;
    else if (strcmp(opstr, "filelist") == 0)
        dm->operation = OPER_FILELIST;
    else if (strcmp(opstr, "publish") == 0)
        dm->operation = OPER_PUBLISH;
    else if (strcmp(opstr, "pingok") == 0)
        dm->operation = OPER_PINGOK;
    else if (strcmp(opstr, "pingbad") == 0)
        dm->operation = OPER_PINGBAD;
    else if (strcmp(opstr, "filelistres") == 0)
        dm->operation = OPER_FILELISTRES;
    else if (strcmp(opstr, "publishack") == 0)
        dm->operation = OPER_PUBLISHACK;
    else {
        dm->operation = OPER_INVALID;
        NF_TRY(
            1, "get_value", "opereration invalid",
        );
    }

    return dm;
}

/* only messages with fields */

void
dm_deserialize_ping(dir_message_t *dm, const char *datagram)
{
    const char *protocolid = NULL;

    NF_TRY_C(
        !(protocolid = get_value("protocol", datagram)),
        "get_value", "key not found", "protocol", return
    );

    dm->data = malloc(sizeof(dir_message_ping_t));
    dm->size = sizeof(dir_message_ping_t);
    ((dir_message_ping_t*)dm->data)->protocolid = strdup(protocolid);
}

void
dm_deserialize_publish(dir_message_t *dm, const char *datagram)
{
    dm->data = malloc(sizeof(dir_message_publish_t));
    dm->size = sizeof(dir_message_publish_t);
    dir_message_publish_t *dmp = (dir_message_publish_t*)dm->data;
    dmp->filelist = filedb_new();

    const char *ptr = strchr(datagram, '\n'); /* skip first line */
    const char *tokend = NULL;

    NF_TRY_C(
        !ptr,
        "strchr", "protocol deserialize error", "expected trailing line feed",
        return
    );

    while (*ptr) {
        ptr++;
        if (!*ptr)
            return;
        tokend = strchr(ptr, ':');

        NF_TRY_C(
            !tokend,
            "strchr", "protocol deserialize error", "expected `:`",
            return
        );

        const char *hash = strndup(ptr, tokend - ptr);
        ptr = strip(tokend + 1);
        tokend = strchr(ptr, ';');

        NF_TRY_C(
            !tokend,
            "strchr", "protocol deserialize error", "expected `;`",
            return
        );

        const char *name = strndup(ptr, tokend - ptr);
        ptr = strip(tokend + 1);
        tokend = strchr(ptr, '\n');

        NF_TRY_C(
            !tokend,
            "strchr", "protocol deserialize error",
            "expected trailing line feed",
            return
        );

        size_t size = strtoll(ptr, NULL, 0);

        filedb_insert(dmp->filelist, name, hash, size);
        ptr = strchr(ptr, '\n');
    }
}

void
dm_deserialize_filelistres(dir_message_t *dm, const char *datagram)
{
    dm->data = malloc(sizeof(dir_message_publish_t));
    dm->size = sizeof(dir_message_publish_t);
    dir_message_publish_t *dmp = (dir_message_publish_t*)dm->data;
    dmp->filelist = filedb_new();

    const char *ptr = strchr(datagram, '\n'); /* skip first line */
    const char *tokend = NULL;

    NF_TRY_C(
        !ptr,
        "strchr", "protocol deserialize error", "expected trailing line feed",
        return
    );

    while (*ptr) {
        ptr++;
        if (!*ptr)
            return;
        tokend = strchr(ptr, ':');

        NF_TRY_C(
            !tokend,
            "strchr", "protocol deserialize error", "expected `:`",
            return
        );

        const char *hash = strndup(ptr, tokend - ptr);
        ptr = strip(tokend + 1);
        tokend = strchr(ptr, ';');

        NF_TRY_C(
            !tokend,
            "strchr", "protocol deserialize error", "expected `;`",
            return
        );

        const char *name = strndup(ptr, tokend - ptr);
        ptr = strip(tokend + 1);
        tokend = strchr(ptr, ';');

        NF_TRY_C(
            !tokend,
            "strchr", "protocol deserialize error", "expected `;`",
            return
        );

        size_t size = strtoll(ptr, NULL, 0);

        ptr = strip(tokend + 1);

        file_info_t *fi = filedb_insert(dmp->filelist, name, hash, size);
        fi->serverlist = sl_new();

        while (*ptr) {
            tokend = strpbrk(ptr, ",\n");

            NF_TRY_C(
                !tokend,
                "strpbrk", "protocol deserialize error",
                "expected , or trailing line feed",
                return
            );

            if (tokend - ptr) {
                sl_insert(fi->serverlist, strndup(ptr, tokend - ptr));
            } else {
                NF_TRY_C(
                    1,
                    "strpbrk", "protocol deserialize warning",
                    "zero-length hostname",
                );
            }

            if (*tokend == '\n')
                break;
            ptr = strip(tokend + 1);
        }

        ptr = strchr(ptr, '\n');
    }
}

void
dm_destroy(dir_message_t *dm)
{
    free(dm);
}

