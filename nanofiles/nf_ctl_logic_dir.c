#include "nf_ctl_logic_dir.h"

#include "../common/dir_message.h"
#include "../common/filedb.h"

#include <stdlib.h>
#include <stdio.h>

logicdir_t*
logicdir_new(const char *directory_hostname)
{
    logicdir_t *ld = malloc(sizeof(logicdir_t));

    ld->dc = dc_new(directory_hostname);
    ld->directory_hostname = directory_hostname;

    return ld;
}

int
logicdir_test(logicdir_t *ld)
{
    printf("testing connection to directory... ");
    fflush(stdout);

    if (dc_test(ld->dc)) {
        printf("online\n");
        return 1;
    } else {
        printf("offline\n");
        return 0;
    }

}

int
logicdir_register_server(logicdir_t *ld, filedb_t *db)
{
    /* register server */
    printf("registering... ");
    fflush(stdout);
    /* generate request*/
    const char *request = dm_publish(db);
    /* perform request */
    const char *response = dc_request(ld->dc, request);
    /* deserialize response */
    dir_message_t *dm = dm_deserialize(response);
    if (dm && dm->operation == OPER_PUBLISHACK) {
        printf("ok\n");
        return 1;
    } else {
        printf("failed\n");
        return 0;
    }
}

int
logicdir_fetch_print_files(logicdir_t *ld)
{
    /* generate request */
    const char *request = dm_filelist();
    /* perform request */
    const char *response = dc_request(ld->dc, request);
    if (!response)
        return 0;
    /* deserialize response */
    dir_message_t *dm = dm_deserialize(response);
    if (!dm)
        return 0;
    dm_deserialize_filelistres(dm, response);
    dir_message_filelistres_t *dmf = 
        (dir_message_filelistres_t*)dm->data;
    /* print results */
    filedb_print(dmf->db, stdout);
    /* clean up */
    filedb_destroy(dmf->db);
    return 1;
}

int
logicdir_ping(logicdir_t *ld)
{
    printf("trying directory at %s... ", ld->directory_hostname);
    fflush(stdout);

    if (dc_ping_raw(ld->dc)) {
        printf("ok\n");
        return 1;
    } else {
        printf("failed: no answer or wrong protocol\n");
        return 0;
    }
}

void
logicdir_destroy(logicdir_t *ld)
{
    dc_destroy(ld->dc);
}

