#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>

#include "config.h"
#include "interface.h"
#include "device.h"
#include "handler.h"

extern device_t devices[];

enum { LED, BUZZER, CDS, FND };

pthread_mutex_t device_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int device_index;
    int cmd;
    void *param;
    pthread_mutex_t *mutex;
} device_task_t;

int accept_client(int server_fd, int *client_fd)
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (*client_fd < 0)
    {
        syslog(LOG_ERR, "accept() failed: %s", strerror(errno));
        return ACCEPT_FAILED;
    }

    char addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, sizeof(addr_str));
    syslog(LOG_INFO, "Client connected: %s:%d", addr_str, ntohs(client_addr.sin_port));

    return 0;
}

int receive_command(int client_fd, char *recv_buf, size_t buf_size)
{
    ssize_t received = recv(client_fd, recv_buf, buf_size - 1, 0);
    if (received <= 0)
    {
        syslog(LOG_ERR, "recv() failed or connection closed: %s", strerror(errno));
        return RECV_FAILED;
    }
    recv_buf[received] = '\0';
    syslog(LOG_INFO, "Received command: %s", recv_buf);
    return 0;
}

void *device_control_thread(void *arg)
{
    device_task_t *task = (device_task_t *)arg;

    if (task->mutex) pthread_mutex_lock(task->mutex);

    int result = devices[task->device_index].control(task->cmd, task->param);
    syslog(LOG_INFO, "Device %d control cmd %d result: %d", task->device_index, task->cmd, result);

    if (task->mutex) pthread_mutex_unlock(task->mutex);

    if (task->param) free(task->param);
    free(task);

    return NULL;
}

int process_command(const char *recv_buf, int *cmd_out)
{
    if (strncmp(recv_buf, "CMD:", 4) != 0)
    {
        syslog(LOG_WARNING, "Invalid command prefix");
        return STRNCMP_FAILED;
    }

    int cmd_num = -1;
    int param = -1;

    char *cmd_copy = strdup(recv_buf);
    if (!cmd_copy)
    {
        syslog(LOG_ERR, "strdup() failed");
        return STRDUP_FAILED;
    }

    char *token = strtok(cmd_copy + 4, ":");
    if (token)
    {
        cmd_num = atoi(token);
        token = strtok(NULL, ":");
        if (token) param = atoi(token);
    }
    free(cmd_copy);

    if (cmd_num < 0 || cmd_num > 6)
    {
        syslog(LOG_WARNING, "Command number out of range: %d", cmd_num);
        return CMD_NUM_OUT_OF_RANGE;
    }

    if (cmd_num == 0)
    {
        *cmd_out = 0;
        return 0;
    }

    device_task_t *task = malloc(sizeof(device_task_t));
    if (!task)
    {
        syslog(LOG_ERR, "malloc() for device_task_t failed");
        return MALLOC_TASK_FAILED;
    }

    task->param = NULL;
    task->mutex = &device_mutex;

    switch (cmd_num)
    {
        case 1: task->device_index = LED;    task->cmd = LED_ON;  break;
        case 2: task->device_index = LED;    task->cmd = LED_OFF; break;
        case 3: task->device_index = BUZZER; task->cmd = BUZZER_ON; break;
        case 4: task->device_index = BUZZER; task->cmd = BUZZER_OFF; break;
        case 5: task->device_index = CDS;    task->cmd = CDS_READ; break;
        case 6:
            if (param < 0 || param > 9)
            {
                syslog(LOG_WARNING, "FND parameter out of range: %d", param);
                free(task);
                return PARAM_OUT_OF_RANGE;
            }
            task->device_index = FND;
            task->cmd = FND_COUNTDOWN;
            task->param = malloc(sizeof(int));
            if (!task->param)
            {
                syslog(LOG_ERR, "malloc() for FND param failed");
                free(task);
                return MALLOC_PARAM_FAILED;
            }
            *((int *)task->param) = param;
            break;
    }

    pthread_t tid;
    if (pthread_create(&tid, NULL, device_control_thread, task) != 0)
    {
        syslog(LOG_ERR, "pthread_create() failed");
        if (task->param) free(task->param);
        free(task);
        return PTHREAD_CREATE_FAILED;
    }

    pthread_detach(tid);
    *cmd_out = cmd_num;

    return 0;
}

int send_response(int client_fd, const char *send_buf)
{
    size_t len = strlen(send_buf);
    ssize_t total_sent = 0;

    while (total_sent < len)
    {
        ssize_t sent = write(client_fd, send_buf + total_sent, len - total_sent);
        if (sent < 0)
        {
            syslog(LOG_ERR, "write() failed: %s", strerror(errno));
            close(client_fd);
            return SEND_FAILED;
        }
        total_sent += sent;
    }

    syslog(LOG_INFO, "Sent response: %s", send_buf);
    return 0;
}

void close_client(int client_fd)
{
    if (client_fd >= 0)
    {
        syslog(LOG_INFO, "Client disconnected (fd: %d)", client_fd);
        close(client_fd);
    }
}
