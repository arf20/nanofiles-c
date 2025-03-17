#ifndef _NETUTIL_H
#define _NETUTIL_H

#include <sys/socket.h>

#define NF_TRY(f, fname, err, a)  if ((f)) { \
    fprintf(stderr, "error: " __FILE__ ":%d: %s: %s\n", \
        __LINE__, (fname), (err)); \
    a; \
} 

struct sockaddr resolve_name(const char *name);
socklen_t sa_len(const struct sockaddr *sa);
int sock_set_timeout(int fd, int timeoutsec);

#endif /* _NETUTIL_H */

