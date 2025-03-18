#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "directory_connector.h"

void
usage(const char *argv0)
{
    fprintf(stderr,
        "usage: %s [-d <shared directory>] [-s <directory hostname>]\n",
        argv0);
    exit(1);
}

int
main(int argc, char **argv)
{
    const char *directory_hostname = DEFAULT_DIRECTORY_HOSTNAME;
    const char *shared_dirname = DEFAULT_SHARED_DIRNAME;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || i + 1 >= argc)
            usage(argv[0]);

        if (argv[i][1] == 'd') {
            directory_hostname = argv[i + 1];
            i++;
        } else if (argv[i][1] == 's') {
            shared_dirname = argv[i + 1];
            i++;
        } else
            usage(argv[0]);
    }

}

