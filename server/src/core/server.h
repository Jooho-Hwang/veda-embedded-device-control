#ifndef SERVER_H
#define SERVER_H

int daemonize(const char *);
int init_server_resource(void);
void loop_server(void);
void close_server(void);

#endif // SERVER_H