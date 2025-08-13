#ifndef HANDLER_H
#define HANDLER_H

#include <stddef.h>

#include "device.h"

int accept_client(int, int *);
int receive_command(int, char *, size_t);
int process_command(const char *, int *);
int send_response(int, const char *);
void close_client(int);

#endif // HANDLER_H