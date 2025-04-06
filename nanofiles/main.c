#include "../common/config.h"
#include "../common/filedb.h"

#include "nf_config.h"
#include "directory_connector.h"
#include "nf_controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
usage(const char *argv0)
{
    fprintf(stderr,
        "usage: %s [-tu|-tt|-d <dir>|-s <hostname>]\n"
        "\t-tu:\ttest mode udp\n"
        "\t-tt:\ttest mode tcp\n"
        "\t-d host:\tdirectory server hostname\n"
        "\t-s path:\tshared path\n",
        argv0);
    exit(1);
}

int
main(int argc, char **argv)
{
    const char *directory_hostname = DEFAULT_DIRECTORY_HOSTNAME;
    const char *shared_dir = DEFAULT_SHARED_DIRNAME;
    int test_mode_udp = 0, test_mode_tcp = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 >= argc) usage(argv[0]);
            directory_hostname = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) usage(argv[0]);
            shared_dir = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-tu") == 0) {
            test_mode_udp = 1;
        } else if (strcmp(argv[i], "-tt") == 0) {
            test_mode_tcp = 1;
        } else
            usage(argv[0]);
    }

    printf("scanning %s...\n", shared_dir);

    filedb_t *db = filedb_new();
    if (filedb_scan(db, shared_dir, "") < 0)
        return 1;

    ctl_t *ctl = ctl_new(db, directory_hostname, shared_dir);
    if (!ctl)
        return 1;

    if (test_mode_udp) {
        ctl_test_directory(ctl);
        return 0;
    }

    do {
        ctl_process_command(ctl, test_mode_tcp);
    } while (!ctl->quit);

    ctl_destroy(ctl);

    return 0;
}

