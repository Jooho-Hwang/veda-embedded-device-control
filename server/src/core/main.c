#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>

#include "config.h"
#include "server.h"

static void handle_signal(int signo)
{
    syslog(LOG_INFO, "Received signal %d, shutting down server", signo);
    close_server();
    exit(0);
}

int main(void)
{
    openlog(HJH_SERVER, LOG_PID | LOG_CONS, LOG_DAEMON);

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        syslog(LOG_ERR, "Failed to ignore SIGPIPE");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGINT, handle_signal) == SIG_ERR)
    {
        syslog(LOG_ERR, "Failed to handle SIGINT");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGTERM, handle_signal) == SIG_ERR)
    {
        syslog(LOG_ERR, "Failed to handle SIGTERM");
        exit(EXIT_FAILURE);
    }

    int check_daemon = daemonize(HJH_SERVER);
    if (check_daemon < 0)
    {
        syslog(LOG_ERR, "Daemonize failed with code %d", check_daemon);
        exit(EXIT_FAILURE);
    }

    int check_server_resource = init_server_resource();
    if (check_server_resource < 0)
    {
        syslog(LOG_ERR, "Server resource initialization failed with code %d", check_server_resource);
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Server started successfully");

    loop_server();

    syslog(LOG_INFO, "Server loop exited normally");
    closelog();
    return 0;
}
