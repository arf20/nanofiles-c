#include "nf_controller.h"

#include "../common/dir_message.h"
#include "nf_shell.h"

#include <stdlib.h>
#include <stdio.h>

ctl_t*
ctl_new(filedb_t *db, const char *directory_hostname)
{
    ctl_t *ctl = malloc(sizeof(ctl_t));

    ctl->state = OFFLINE;
    ctl->dc = dc_new(directory_hostname);
    ctl->shell = shell_new();
    ctl->db = db;
    ctl->quit = 0;
    ctl->directory_hostname = directory_hostname;

    return ctl;
}

void
ctl_test_directory(ctl_t *ctl)
{
    printf("testing connection to directory... ");
    fflush(stdout);

    if (dc_test(ctl->dc)) {
        printf("online\n");
    } else {
        printf("offline\n");
    }

}

void
ctl_process_command(ctl_t *ctl)
{
    cmd_arg_t cmd = shell_read_command(ctl->shell);

    switch (cmd.cmd) {
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
        }
        case CMD_LISTREMOTE: {
            /* generate request */
            const char *request = dm_filelist();
            /* perform request */
            const char *response = dc_request(ctl->dc, request);
            if (!response)
                return;
            /* deserialize response */
            dir_message_t *dm = dm_deserialize(response);
            if (!dm)
                return;
            dm_deserialize_filelistres(dm, response);
            dir_message_filelistres_t *dmf = 
                (dir_message_filelistres_t*)dm->data;
            /* print results */
            filedb_print(dmf->db, stdout);
            /* clean up */
            filedb_destroy(dmf->db);
        } break;
        case CMD_LISTLOCAL: {
            filedb_print(ctl->db, stdout);
        } break;
        case CMD_SERVE: {
            /* register server */
            printf("registering... ");
            fflush(stdout);
            /* generate request*/
            const char *request = dm_publish(ctl->db);
            /* perform request */
            const char *response = dc_request(ctl->dc, request);
            /* deserialize response */
            dir_message_t *dm = dm_deserialize(response);
            if (dm && dm->operation == OPER_PUBLISHACK)
                printf("ok\n");
            else {
                printf("failed\n");
                return;
            }

            /* TODO: peer server */
        } break;
        case CMD_PING: {
            printf("trying directory at %s... ", ctl->directory_hostname);
            fflush(stdout);

            if (dc_ping_raw(ctl->dc)) {
                ctl->state = ONLINE;
                printf("ok\n");
            } else {
                ctl->state = OFFLINE;
                printf("bad: no answer or wrong protocol\n");
            }
        } break;
        case CMD_DOWNLOAD: {

        } break;
        case CMD_UPLOAD: {

        } break;
    }
}

void
ctl_destroy(ctl_t *ctl)
{
    shell_destroy(ctl->shell);
    dc_destroy(ctl->dc);
}

