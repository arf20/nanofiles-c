#include "netutil.h"

#include "util.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <netdb.h>

#include <arpa/inet.h>

int
resolve_name(const char *name, struct sockaddr *out_addr)
{
    struct addrinfo *ai = NULL;
    int res = 0;
    NF_TRY(
        (res = getaddrinfo(name, NULL, NULL, &ai)) != 0,
        "getaddrinfo", gai_strerror(res), return 0
    );
    
    if (!ai)
        return 0;

    /* return first address */
    memcpy(out_addr, ai->ai_addr, ai->ai_addrlen);

    char addr_str_buff[INET6_ADDRSTRLEN];
    if (ai->ai_family == AF_INET6) {
        inet_ntop(AF_INET6, &(((struct sockaddr_in6*)out_addr)->sin6_addr),
            addr_str_buff, INET6_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET, &(((struct sockaddr_in*)out_addr)->sin_addr),
            addr_str_buff, INET6_ADDRSTRLEN);
    }
    printf("resolved %s as %s\n", name, addr_str_buff);

    freeaddrinfo(ai);

    return 1;
}


socklen_t
sa_len(const struct sockaddr *sa)
{
    switch (sa->sa_family) {
        case 0: return 0; break;
        case AF_INET: return sizeof(struct sockaddr_in); break;
        case AF_INET6: return sizeof(struct sockaddr_in6); break;
        default: return 0; break;
    }
}

int
sock_set_timeout(int fd, int timeoutsec)
{
    struct timeval tv;
    tv.tv_sec = timeoutsec;
    tv.tv_usec = 0;
    NF_TRY(
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv))
            < 0,
        "setsockopt", strerror(errno), return -1
    );

    return 0;
}

