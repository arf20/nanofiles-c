#include "nf_controller.h"

#include "../common/util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

ctl_t*
ctl_new(filedb_t *db, const char *directory_hostname, const char *shared_dir)
{
    ctl_t *ctl = malloc(sizeof(ctl_t));

    ctl->state = OFFLINE;
    ctl->ld = logicdir_new(directory_hostname);
    if (!ctl->ld)
        return NULL;
    ctl->lp = logicp2p_new(shared_dir, db);
    ctl->shell = shell_new();
    ctl->db = db;
    ctl->quit = 0;
    ctl->directory_hostname = directory_hostname;
    ctl->shared_dir = shared_dir;

    return ctl;
}

void
ctl_test_directory(ctl_t *ctl)
{
    logicdir_test(ctl->ld);
}

void
ctl_ping(ctl_t *ctl)
{
    ctl->state = logicdir_ping(ctl->ld) ? ONLINE : OFFLINE;
}

void
ctl_help()
{
    printf(
        "\thelp\t\tshow this message\n"
        "\tlistremote\tquery directory files\n"
        "\tlistlocal\tlist files\n"
        "\tserve\t\tserve file\n"
        "\tping\t\tping directory\n"
        "\tdownload\tdownload file\n"
        "\tupload\t\tupload file\n");
}

void
ctl_quit(ctl_t *ctl)
{
    ctl->quit = 1;
}

void
ctl_listlocal(ctl_t *ctl)
{
    filedb_print(ctl->db, stdout);
}

void
ctl_listremote(ctl_t *ctl)
{
    /* fetch directory files and servers */
    if (ctl->state != ONLINE) {
        printf("offline\n");
        return;
    }
    filedb_t *files = logicdir_fetch(ctl->ld);
    if (!files) {
        printf("could not contact directory\n");
        return;
    }
    /* print */
    filedb_print(files, stdout);
    /* clean up */
    filedb_destroy(files);
}

void
ctl_serve(ctl_t *ctl, int test_mode_tcp)
{
    if (ctl->state != ONLINE) {
        printf("offline\n");
        return;
    }
    if (!logicdir_register_server(ctl->ld, ctl->db))
        return;

    if (test_mode_tcp) {
        logicp2p_test_server(ctl->lp);
    } else {
        logicp2p_start_server(ctl->lp);
    }
}

void
ctl_download(ctl_t *ctl, cmd_arg_t *cmd)
{
    /* fetch files */
    if (ctl->state != ONLINE) {
        printf("offline\n");
        return;
    }
    filedb_t *files = logicdir_fetch(ctl->ld);
    if (!files) {
        printf("could not contact directory\n");
        return;
    }
    /* find file */
    file_info_t *file = NULL;
    if (is_sha1(cmd->arg))
        file = filedb_find_hash(files, cmd->arg);
    else
        file = filedb_find_name(files, cmd->arg);
    
    if (!file) {
        printf("file or hash %s is not known by directory\n", cmd->arg);
        return;
    }
    /* download file */
    /*    open output file */
    char output_path[4096];
    snprintf(output_path, 4096, "%s%s", ctl->shared_dir, file->name);
    FILE *output = fopen(output_path, "wb");
    NF_TRY(!output, "fopen", strerror(errno), return);
    
    logicp2p_download(ctl->lp, file, output);
    
    fclose(output);
}

void
ctl_process_command(ctl_t *ctl, int test_mode_tcp)
{
    cmd_arg_t cmd = shell_read_command(ctl->shell);

    switch (cmd.cmd) {
        case CMD_INVALID: return; break;
        case CMD_HELP: ctl_help(); break;
        case CMD_QUIT: ctl_quit(ctl); break;
        case CMD_LISTREMOTE: ctl_listremote(ctl); break;
        case CMD_LISTLOCAL: ctl_listlocal(ctl); break;
        case CMD_SERVE: ctl_serve(ctl, test_mode_tcp); break;
        case CMD_PING: ctl_ping(ctl); break;
        case CMD_DOWNLOAD: ctl_download(ctl, &cmd); break;
        case CMD_UPLOAD: break;
    }
}

void
ctl_destroy(ctl_t *ctl)
{
    shell_destroy(ctl->shell);
    logicp2p_destroy(ctl->lp);
    logicdir_destroy(ctl->ld);
    free(ctl);
}

