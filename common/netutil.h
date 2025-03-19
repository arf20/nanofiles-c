#ifndef _NETUTIL_H
#define _NETUTIL_H

#include <sys/socket.h>

struct sockaddr resolve_name(const char *name);
socklen_t sa_len(const struct sockaddr *sa);
int sock_set_timeout(int fd, int timeoutsec);

#endif /* _NETUTIL_H */

