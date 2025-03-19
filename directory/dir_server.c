#include "dir_server.h"

#include <stdio.h>
#include <stdlib.h>

dir_server_t*
dir_server_new(float corruption_prob)
{
    dir_server_t *ds = malloc(sizeof(dir_server_t));
    ds->corruption_prob = corruption_prob;

    return ds;
}

