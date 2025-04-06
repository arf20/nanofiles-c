#include "nf_controller.h"

#include "../common/util.h"

#include <stdlib.h>
#include <stdio.h>

ctl_t*
ctl_new(filedb_t *db, const char *directory_hostname)
{
    ctl_t *ctl = malloc(sizeof(ctl_t));

    ctl->state = OFFLINE;
    ctl->ld = logicdir_new(directory_hostname);
    if (!ctl->ld)
        return NULL;
    ctl->lp = logicp2p_new();
    ctl->shell = shell_new();
    ctl->db = db;
    ctl->quit = 0;
    ctl->directory_hostname = directory_hostname;

    return ctl;
}

void
ctl_test_directory(ctl_t *ctl)
{
    logicdir_test(ctl->ld);
}

void
ctl_process_command(ctl_t *ctl, int test_mode_tcp)
{
    cmd_arg_t cmd = shell_read_command(ctl->shell);

    switch (cmd.cmd) {
        case CMD_INVALID: return; break;
        case CMD_HELP: {
            printf(
                "\thelp\t\tshow this message\n"
                "\tlistremote\tquery directory files\n"
                "\tlistlocal\tlist files\n"
                "\tserve\t\tserve file\n"
                "\tping\t\tping directory\n"
                "\tdownload\tdownload file\n"
                "\tupload\t\tupload file\n");
        } break;
        case CMD_QUIT: {
            ctl->quit = 1;
        } break;
        case CMD_LISTREMOTE: {
            /* fetch directory files and servers */
            if (ctl->state != ONLINE) {
                printf("offline\n");
                break;
            }
            filedb_t *files = logicdir_fetch(ctl->ld);
            if (!files) {
                printf("could not contact directory\n");
                break;
            }
            /* print */
            filedb_print(files, stdout);
            /* clean up */
            filedb_destroy(files);
        } break;
        case CMD_LISTLOCAL: {
            filedb_print(ctl->db, stdout);
        } break;
        case CMD_SERVE: {
            if (ctl->state != ONLINE) {
                printf("offline\n");
                break;
            }
            if (!logicdir_register_server(ctl->ld, ctl->db))
                break;

            if (test_mode_tcp) {
                logicp2p_test_server(ctl->lp);
            } else {
                logicp2p_start_server(ctl->lp);
            }
        } break;
        case CMD_PING: {
            ctl->state = logicdir_ping(ctl->ld) ? ONLINE : OFFLINE;
        } break;
        case CMD_DOWNLOAD: {
            /* fetch files */
            if (ctl->state != ONLINE) {
                printf("offline\n");
                break;
            }
            filedb_t *files = logicdir_fetch(ctl->ld);
            if (!files) {
                printf("could not contact directory\n");
                break;
            }
            /* find file */
            file_info_t *file = NULL;
            if (is_sha1(cmd.arg))
                file = filedb_find_hash(files, cmd.arg);
            else
                file = filedb_find_name(files, cmd.arg);
            
            if (!file) {
                printf("file or hash %s is not known by directory\n",
                    cmd.arg);
                break;
            }
            /* download file */
            logicp2p_download(ctl->lp, file);
            
        } break;
        case CMD_UPLOAD: {

        } break;
    }
}

void
ctl_destroy(ctl_t *ctl)
{
    shell_destroy(ctl->shell);
    free(ctl);
}

