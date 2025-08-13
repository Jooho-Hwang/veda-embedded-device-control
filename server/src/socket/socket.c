#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>

#include "config.h"
#include "socket.h"

int init_socket(uint16_t _port, int _backlog)
{
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        syslog(LOG_ERR, "socket() failed");
        return SOCKET_FAILED;
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        syslog(LOG_ERR, "setsockopt() failed");
        close(sockfd);
        return SETSOCKETOPT_FAILED;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        syslog(LOG_ERR, "bind() failed on port %d", _port);
        close(sockfd);
        return BIND_FAILED;
    }

    if (listen(sockfd, _backlog) == -1)
    {
        syslog(LOG_ERR, "listen() failed");
        close(sockfd);
        return LISTEN_FAILED;
    }

    syslog(LOG_INFO, "Socket initialized and listening on port %d", _port);
    return sockfd;
}

void close_socket(int _sockfd)
{
    if (_sockfd >= 0)
    {
        close(_sockfd);
        syslog(LOG_INFO, "Socket closed (fd: %d)", _sockfd);
    }
}
