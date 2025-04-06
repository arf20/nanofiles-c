#ifndef _NETUTIL_H
#define _NETUTIL_H

#include <sys/socket.h>

int resolve_name(const char *name, struct sockaddr *out_addr);
socklen_t sa_len(const struct sockaddr *sa);
int sock_set_timeout(int fd, int timeoutsec);

#endif /* _NETUTIL_H */

