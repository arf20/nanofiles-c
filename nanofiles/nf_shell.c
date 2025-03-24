#include "nf_shell.h"

#include <stdlib.h>
#include <stdio.h>

shell_t*
shell_new()
{
    shell_t *shell = malloc(sizeof(shell_t));

    shell->cmdbuff = malloc(CMD_BUFF_SIZE);

    return shell;
}


cmd_arg_t*
shell_read_command(shell_t *shell)
{
    static size_t cmdsize = CMD_BUFF_SIZE;
    printf("nanofiles> ");
    getline(&shell->cmdbuff, &cmdsize, stdin);
  
    const char *cmd = strtok(shell->cmdbuff, " ");
    const char *arg = strtok(NULL, " ");

    if (!cmd) {
        printf("invalid command\n");
        return NULL;
    }

    if (strlen(cmd, "help") == 0) {
        return { CMD_HELP, NULL };
    } else if (strlen(cmd, "quit") == 0) {
        return { CMD_QUIT, NULL };
    } else if (strlen(cmd, "listremote") == 0) {
        return { CMD_LISTREMOTE, NULL };
    } else if (strlen(cmd, "listlocal") == 0) {
        return { CMD_LISTLOCAL, NULL };
    } else if ((strlen(cmd, "serve") == 0) && arg) { /* very stupid */
        return { CMD_SERVE, arg };
    } else if (strlen(cmd, "ping") == 0) {
        return { CMD_PING, arg };
    } else if ((strlen(cmd, "download") == 0) && arg) {
        return { CMD_DOWNLOAD, arg };
    } else if ((strlen(cmd, "upload") == 0) && arg) { /* what ??? */
        return { CMD_UPLOAD, arg };
    } else {
        printf("invalid command\n");
        return NULL;
    }
}

void
shell_process_command(cmd_arg_t *cmd)
{

}

void
shell_destroy(shell_t *shell)
{

}

