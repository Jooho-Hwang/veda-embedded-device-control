#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>

int init_socket(uint16_t, int);
void close_socket(int);

#endif // SOCKET_H