#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Sockets libraries
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// My sockets library
#include "sockets.h"

// Constant values
#define MAX_QUEUE 5
#define BUFFER_SIZE 200
#define DEFAULT_PORT 8989

// Function declarations
void usage(char * program);
void waitForConnections(int server_fd);
void communicationLoop(int connection_fd);

int main(int argc, char * argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
    }
    
    int server_fd;

    srand(time(NULL));
    
    server_fd = prepareServer(argv[1], MAX_QUEUE);
    
    // Start waiting for incoming connections
    waitForConnections(server_fd);
    
    return 0;
}

void usage(char * program)
{
    printf("Usage: %s {port_number}\n", program);
    exit(EXIT_SUCCESS);
}

void waitForConnections(int server_fd)
{
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof client_address;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pid_t new_pid;
    
    while(1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_addrlen);
        if (client_fd == -1)
        {
            close(server_fd);
            close(client_fd);
            perror("ERROR: listen");
        }

        new_pid = fork();
        
        if (new_pid == 0)   // Child process
        {
            // Close the main port
            close(server_fd);
            inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, INET_ADDRSTRLEN); 
            printf("Connection from: %s, port %i\n", client_presentation, client_address.sin_port);
            
            // Start the communication loop
            communicationLoop(client_fd);
            
            // Terminate the child process
            close(client_fd);
            exit(EXIT_SUCCESS);
        }
        else if (new_pid > 0)   // Parent process
        {
            close(client_fd);
        }
        else
        {
            perror("ERROR: fork");
            exit(EXIT_FAILURE);
        }
    }
    
    // Close the server port
    close(server_fd);
}

void communicationLoop(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    int chars_read = 0;
    int range = 1000;
    int secret = rand() % range + 1;
    int num;
    
    printf("Number to guess. %d\n", secret);
    
    // Handshake
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    send(connection_fd, "READY", strlen("READY")+1, 0);
    
    while(1)
    {
        //Generate random cards for dealer and player
        int 

        sprintf(buffer, "Your cards:\n");



        // Get the guess from the user
        chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
        if (chars_read <= 0)
            break;
        // Convert into a number
        sscanf(buffer, "%d", &num);
        printf("Got the number %d\n", num);
       
        // Prepare the response
        if (num > secret)
            sprintf(buffer, "Try a lower number");
        if (num < secret)
            sprintf(buffer, "Try a higher number");
        if (num == secret)
            sprintf(buffer, "Right");
        send(connection_fd, buffer, strlen(buffer)+1, 0);
    }
    
    // Close the socket to the client
    close(connection_fd);
}