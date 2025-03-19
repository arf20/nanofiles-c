#include "../common/config.h"
#include "dir_config.h"

#include "dir_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
usage(const char *argv0)
{
    fprintf(stderr,
        "usage: %s [-loss <prob>]\n"
        "\t-loss <prob>:\tCorruption probability\n",
        argv0);
    exit(1);
}

int
main(int argc, char **argv)
{
    float corruption_prob = DEFAULT_CORRUPTION_PROBABILITY;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-loss") == 0) {
            if (i + 1 >= argc) usage(argv[0]);
            corruption_prob = atof(argv[i + 1]);
            i++;
        } else
            usage(argv[0]);
    }

    printf("corruption probability: %f\n", corruption_prob); 

    dir_server_t *ds = dir_server_new(corruption_prob);


    return 0;
}

