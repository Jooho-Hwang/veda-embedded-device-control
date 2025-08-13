#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <syslog.h>

#include "config.h"
#include "server.h"
#include "device.h"
#include "socket.h"
#include "handler.h"

static int server_fd = -1;

int daemonize(const char *_server_name)
{
	(void)_server_name;
    struct sigaction sa;
    struct rlimit rl;
	pid_t pid;
    int fd0, fd1, fd2;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
	{
		return GETRLMINIT_FAILED;
    }

    if ((pid = fork()) < 0)
	{
		return FORK_FAILED;
    }
	else if (pid != 0)
	{
        exit(0);
    }

    if (setsid() < 0)
	{
        return SETSID_FAILED;
    }

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
	{
        return SIGACTION_FAILED;
    }

    if (chdir("/") < 0)
	{
        return CHDIR_FAILED;
    }

    if (rl.rlim_max == RLIM_INFINITY) rl.rlim_max = 1024;
    for (int i = 0; i < rl.rlim_max; i++) close(i);

    fd0 = open("/dev/null", O_RDONLY);
    if (fd0 < 0)  return STDIN_CON_FAILED;
    if (fd0 != 0) return STDIN_DUP_FAILED;

    fd1 = open("/dev/null", O_WRONLY);
    if (fd1 < 0)  return STDOUT_CON_FAILED;
    if (fd1 != 1) return STDOUT_DUP_FAILED;

    fd2 = open("/dev/null", O_RDWR);
    if (fd2 < 0)  return STDERR_CON_FAILED;
    if (fd2 != 2) return STDERR_DUP_FAILED;
	
    return 0;
}

int init_server_resource(void)
{
    syslog(LOG_INFO, "Loading device libraries...");
    int check_libs = load_device_libs();
    if (check_libs < 0)
    {
        syslog(LOG_ERR, "Failed to load device libraries: %d", check_libs);
        return check_libs;
    }

    syslog(LOG_INFO, "Initializing devices...");
    int check_devices = init_all_devices();
    if (check_devices < 0)
    {
        syslog(LOG_ERR, "Failed to initialize devices: %d", check_devices);
        unload_device_libs(NUM_DEVICES);
        return check_devices;
    }

    syslog(LOG_INFO, "Initializing socket on port %d", PORT);
    int check_socket = init_socket(PORT, BACKLOG);
    if (check_socket < 0)
    {
        syslog(LOG_ERR, "Failed to initialize socket: %d", check_socket);
        cleanup_all_devices();
        unload_device_libs(NUM_DEVICES);
        return check_socket;
    }

    server_fd = check_socket;
    return 0;
}

void loop_server(void)
{
    char recv_buf[BUFF_SIZE];
    char send_buf[BUFF_SIZE];
    int client_fd;

    syslog(LOG_INFO, "Entering server loop");

    while (1)
    {
        if (accept_client(server_fd, &client_fd) < 0)
        {
            syslog(LOG_WARNING, "Failed to accept client");
            sleep(1);
            continue;
        }

        syslog(LOG_INFO, "Client connected (fd=%d)", client_fd);

        while (1)
        {
            memset(recv_buf, 0, sizeof(recv_buf));
            memset(send_buf, 0, sizeof(send_buf));

            if (receive_command(client_fd, recv_buf, sizeof(recv_buf)) < 0)
            {
                syslog(LOG_WARNING, "Failed to receive command from client");
                break;
            }

            int cmd_num = -1;
            int status = process_command(recv_buf, &cmd_num);

            if (status == STRNCMP_FAILED || status == CMD_NUM_OUT_OF_RANGE || status == PARAM_OUT_OF_RANGE)
            {
                snprintf(send_buf, sizeof(send_buf), "Invalid command\n");
                syslog(LOG_WARNING, "Invalid command received: %s", recv_buf);
            }
            else if (cmd_num == 0)
            {
                snprintf(send_buf, sizeof(send_buf), "Goodbye\n");
                syslog(LOG_INFO, "Client requested disconnect");
                send_response(client_fd, send_buf);
                break;
            }
            else
            {
                snprintf(send_buf, sizeof(send_buf), "Command %d accepted\n", cmd_num);
                syslog(LOG_INFO, "Processed command %d", cmd_num);
            }

            if (send_response(client_fd, send_buf) < 0)
            {
                syslog(LOG_WARNING, "Failed to send response to client");
                break;
            }
        }

        close_client(client_fd);
        syslog(LOG_INFO, "Client disconnected (fd=%d)", client_fd);
    }
}

void close_server(void)
{
    syslog(LOG_INFO, "Shutting down server...");

    if (server_fd >= 0)
    {
        close_socket(server_fd);
        server_fd = -1;
    }

    cleanup_all_devices();
    unload_device_libs(NUM_DEVICES);

    syslog(LOG_INFO, "Server resources cleaned up");
}
