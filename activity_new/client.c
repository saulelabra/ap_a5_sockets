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
#define BUFFER_SIZE 200
#define DEFAULT_PORT 8989

// Function declarations
void usage(char * program);
void communicationLoop();

int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        usage(argv[0]);
    }
    
    int connection_fd;
    
    connection_fd = connectToServer(argv[1], argv[2]);
    
    communicationLoop(connection_fd);
    
    return 0;
}

void usage(char * program)
{
    printf("Usage: %s {server_address} {port_number}\n", program);
    exit(EXIT_SUCCESS);
}

void insurance(int connection_fd, char * buffer)
{
    bzero(buffer, BUFFER_SIZE);
    int chars_read;
    char option;

    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);//insurance is possible
    option = buffer[0];
    if(option == '1')
    {
        send(connection_fd, "ACK", strlen("ACK")+1, 0);//ACK insurance possible

        //Print menu
        chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);//menu
        printf("%s", buffer);

        //Send answer
        scanf("%s", buffer);
        printf("option: %s\n", buffer);

        if(buffer == "y")
        {
            send(connection_fd, buffer, strlen(buffer), 0);

            chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);

            printf("%s", buffer);
            
            scanf("%s", buffer);
            send(connection_fd, buffer, strlen(buffer), 0);

            /*chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
            printf("%s", buffer);
            scanf("%s", buffer);
            send(connection_fd, buffer, strlen(buffer), 0);*/
        }else{
            send(connection_fd, buffer, strlen(buffer), 0);
        }
    }else{
        send(connection_fd, "ACK", strlen("ACK")+1, 0);//ACK insurance possible
    }
}

void communicationLoop(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    int chars_read = 0;
    
    // Handshake
    send(connection_fd, "START", strlen("START")+1, 0);
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);

    //Send the bet to the server
    printf("Welcome to Blackjack, type your bet\n");
    scanf("%s", buffer);
    send(connection_fd, buffer, strlen(buffer), 0);

    //Receive and show initial cards
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    printf("%s", buffer);
    send(connection_fd, "ACK", strlen("ACK")+1, 0);

    //if insurance is possible
    insurance(connection_fd, buffer);

    /*//Check if insurance == true
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    printf("Buffer: %s\n", buffer);
    if(buffer[0] == '1')
    {
        char option;

        printf("Do you want to place an insurance bet?\n");
        printf("(y) yes / (n) no\n");
        scanf("%s", &option);
        
        if(option == 'y')
        {
            printf("Type your additional bet\n");
            scanf("%s", buffer);
            send(connection_fd, buffer, strlen(buffer), 0);
        }else{
            send(connection_fd, "ACK", strlen("ACK")+1, 0);
        }
        
    }*/
        
    while(1)
    {
        /*printf("Enter a number: ");
        // Prepare the message
        scanf("%s", buffer);
        send(connection_fd, buffer, strlen(buffer)+1, 0);
        
        // Get the reply from the server
        chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
        if (chars_read <= 0)
            break;
        // Give feedback to the user
        printf("%s\n", buffer);
        // Finish when the game is won
        if (!strncmp(buffer, "Right", 6))
        {
            printf("Congratulations!\n");
            break;
        }*/
    }
    
    // Close the socket to the client
    close(connection_fd);
}