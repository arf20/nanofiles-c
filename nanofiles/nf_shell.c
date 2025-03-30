#include "nf_shell.h"

#include "../common/config.h"
#include "directory_connector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

shell_t*
shell_new()
{
    shell_t *shell = malloc(sizeof(shell_t));
    shell->cmdbuff = malloc(CMD_BUFF_SIZE);
    return shell;
}




cmd_arg_t
shell_read_command(shell_t *shell)
{
    static size_t cmdsize = CMD_BUFF_SIZE;
    printf("nanofiles> ");
    getline(&shell->cmdbuff, &cmdsize, stdin);

    if (strlen(shell->cmdbuff) == 1)
        return (cmd_arg_t){ CMD_INVALID, NULL };
  
    char *cmd = strtok(shell->cmdbuff, " ");
    const char *arg = strtok(NULL, " ");

    cmd[strlen(cmd) - 1] = '\0'; /* strpip trailing \n */

    if (!cmd) {
        printf("invalid command\n");
        return (cmd_arg_t){ CMD_INVALID, NULL };
    }

    if (strcmp(cmd, "help") == 0) {
        return (cmd_arg_t){ CMD_HELP, NULL };
    } else if (strcmp(cmd, "quit") == 0) {
        return (cmd_arg_t){ CMD_QUIT, NULL };
    } else if (strcmp(cmd, "listremote") == 0) {
        return (cmd_arg_t){ CMD_LISTREMOTE, NULL };
    } else if (strcmp(cmd, "listlocal") == 0) {
        return (cmd_arg_t){ CMD_LISTLOCAL, NULL };
    } else if (strcmp(cmd, "serve") == 0) {
        return (cmd_arg_t){ CMD_SERVE, NULL };
    } else if (strcmp(cmd, "ping") == 0) {
        return (cmd_arg_t){ CMD_PING, arg };
    } else if ((strcmp(cmd, "download") == 0) && arg) {
        return (cmd_arg_t){ CMD_DOWNLOAD, arg };
    } else if ((strcmp(cmd, "upload") == 0) && arg) { /* what ??? */
        return (cmd_arg_t){ CMD_UPLOAD, arg };
    } else {
        printf("invalid command\n");
        return (cmd_arg_t){ CMD_INVALID, NULL };
    }
}

void
shell_destroy(shell_t *shell)
{
    free(shell->cmdbuff);
    free(shell);
}

