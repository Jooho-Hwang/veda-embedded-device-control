#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 60000
#define CMD_BUF_SIZE 128
#define RESP_BUF_SIZE 256

volatile sig_atomic_t stop = 0;

void handle_signal(int signo)
{
    (void)signo;
    stop = 1;
}

void setup_signal_handling(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void print_menu(void)
{
    puts("====== Device Control Menu ======");
    puts("1. LED ON");
    puts("2. LED OFF");
    puts("3. Buzzer ON (Play music)");
    puts("4. Buzzer OFF (Stop music)");
    puts("5. Check CDS sensor for 10 seconds");
    puts("6. Run 7-segment timer (0-9)");
    puts("0. Exit");
    puts("==================================");
    printf("Select an option:\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
	{
        fprintf(stderr, "Usage: %s [server_ip]\n", argv[0]);
        return 1;
    }

    setup_signal_handling();

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	{
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
	{
        fprintf(stderr, "Invalid address: %s\n", argv[1]);
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
        perror("connect");
        close(sock);
        return 1;
    }

    while (!stop)
    {
        print_menu();

        char raw_input[CMD_BUF_SIZE] = {0};
        char cmd_buf[CMD_BUF_SIZE] = {0};

        if (!fgets(raw_input, sizeof(raw_input), stdin)) break;
        raw_input[strcspn(raw_input, "\n")] = 0;

        if (strcmp(raw_input, "0") == 0)
		{
            strcpy(cmd_buf, "CMD:0");
        }
        else if (strcmp(raw_input, "6") == 0)
		{
            int param = 0;
            printf("Enter parameter for CMD:9 (0-9): ");
            if (scanf("%d", &param) != 1 || param < 0 || param > 9)
			{
                printf("Invalid parameter. Try again.\n");
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n');
            snprintf(cmd_buf, sizeof(cmd_buf), "CMD:6:%d", param);
        }
        else
		{
            snprintf(cmd_buf, sizeof(cmd_buf), "CMD:%.120s", raw_input);
        }

        if (send(sock, cmd_buf, strlen(cmd_buf), 0) <= 0) break;

        char resp_buf[RESP_BUF_SIZE] = {0};
        int len = recv(sock, resp_buf, sizeof(resp_buf) - 1, 0);
        if (len <= 0) break;

        printf("[Server] %s\n", resp_buf);

        if (strncmp(cmd_buf, "CMD:0", 5) == 0) break;
    }

    close(sock);
    puts("Client terminated.");
    return 0;
}
