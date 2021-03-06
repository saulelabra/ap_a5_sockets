//Saúl Enrique Labra Cruz A01020725
//a5_sockets

#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Sockets libraries
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Function declarations
int prepareServer(char * port, int max_queue);
int connectToServer(char * address, char * port);
int recvMessage(int connection_fd, char * buffer, int buffer_size);

#endif /* SOCKETS_H */