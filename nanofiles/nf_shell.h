#ifndef _NF_SHELL_H
#define _NF_SHELL_H

#include "directory_connector.h"

typedef struct {
    char *cmdbuff;
    dc_t *dc;
    int quit;
} shell_t;

typedef enum {
    CMD_INVALID,
    CMD_HELP,
    CMD_QUIT,
    CMD_LISTREMOTE,
    CMD_LISTLOCAL,
    CMD_SERVE,
    CMD_PING,
    CMD_DOWNLOAD,
    CMD_UPLOAD
} cmd_t;

typedef struct {
    cmd_t cmd;
    const char *arg;
} cmd_arg_t;

shell_t* shell_new();
cmd_arg_t shell_read_command(shell_t *shell);
void shell_destroy(shell_t *shell);

#endif /* _NF_SHELL_H */

