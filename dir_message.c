#include "dir_message.h"

#include "config.h"

#include <stdio.h>
#include <string.h>

static char buff[MAX_DGRAM_SIZE];

const char*
dm_ping(const char *protoid)
{
    snprintf(buff, MAX_DGRAM_SIZE, "ping&%s", NF_PROTOCOL_ID);
    return buff;
}

