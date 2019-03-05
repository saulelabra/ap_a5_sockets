//Saúl Enrique Labra Cruz A01020725
//a5_sockets

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

void getCards(int connection_fd, char * buffer)
{
    bzero(buffer, BUFFER_SIZE);
    recvMessage(connection_fd, buffer, BUFFER_SIZE);
    printf("%s", buffer);
    send(connection_fd, "ACK", strlen("ACK")+1, 0);
}

void insurance(int connection_fd, char * buffer)
{
    bzero(buffer, BUFFER_SIZE);
    char option;

    recvMessage(connection_fd, buffer, BUFFER_SIZE);//insurance is possible
    option = buffer[0];
    if(option == '1')
    {
        send(connection_fd, "ACK", strlen("ACK")+1, 0);//ACK insurance possible

        //Print menu
        recvMessage(connection_fd, buffer, BUFFER_SIZE);//menu
        printf("%s", buffer);

        //Send answer
        scanf("%s", buffer);
        //printf("option: %s\n", buffer);

        if(buffer[0] == 'y')
        {
            send(connection_fd, buffer, strlen(buffer), 0);

            recvMessage(connection_fd, buffer, BUFFER_SIZE);

            printf("%s", buffer);
            
            //send new bet
            scanf("%s", buffer);
            send(connection_fd, buffer, strlen(buffer), 0);
        }else{
            send(connection_fd, buffer, strlen(buffer), 0);
        }
    }else{
        send(connection_fd, "ACK", strlen("ACK")+1, 0);//ACK insurance
    }
}

void checkforwin(int connection_fd, char * buffer)
{
    bzero(buffer, BUFFER_SIZE);
    recvMessage(connection_fd, buffer, BUFFER_SIZE);
    send(connection_fd, "ACK", strlen("ACK")+1, 0);
    getCards(connection_fd, buffer);

    /*//if dealer is natural
    recvMessage(connection_fd, buffer, BUFFER_SIZE);
    send(connection_fd, "ACK", strlen("ACK")+1, 0);
    if(buffer[0] == '1')
    {
        printf("Dealer has a natural!\n");
        getCards(connection_fd, buffer);

        //if player has Blackjack
        recvMessage(connection_fd, buffer, BUFFER_SIZE);
        send(connection_fd, "ACK", strlen("ACK")+1, 0);
        if(buffer[0] == '1')
        {
            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            send(connection_fd, "ACK", strlen("ACK")+1, 0);

            printf("%s", buffer);            
        }else{
            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            send(connection_fd, "ACK", strlen("ACK")+1, 0);

            printf("%s", buffer);  
        }
        recvMessage(connection_fd, buffer, BUFFER_SIZE);
        send(connection_fd, "ACK", strlen("ACK")+1, 0);
        printf("%s", buffer); 

        //if player has insurance bet
        recvMessage(connection_fd, buffer, BUFFER_SIZE);
        send(connection_fd, "ACK", strlen("ACK")+1, 0);
        if(buffer[0] == '1')
        {
            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            send(connection_fd, "ACK", strlen("ACK")+1, 0);

            printf("%s", buffer);  
        }
    }else{
        //if player has insurance bet
        recvMessage(connection_fd, buffer, BUFFER_SIZE);
        send(connection_fd, "ACK", strlen("ACK")+1, 0);
        if(buffer[0] == '1')
        {
            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            send(connection_fd, "ACK", strlen("ACK")+1, 0);

            printf("%s", buffer);
        }

        //if player has Blackjack
        recvMessage(connection_fd, buffer, BUFFER_SIZE);
        send(connection_fd, "ACK", strlen("ACK")+1, 0);
        if(buffer[0] == '1')
        {
            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            send(connection_fd, "ACK", strlen("ACK")+1, 0);

            printf("%s", buffer);
        }else{
            //if player has doubledown
            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            send(connection_fd, "ACK", strlen("ACK")+1, 0);
            if(buffer[0] == '1')
            {
                recvMessage(connection_fd, buffer, BUFFER_SIZE);
                send(connection_fd, "ACK", strlen("ACK")+1, 0);

                printf("%s", buffer);

                //if dealer has finished
                recvMessage(connection_fd, buffer, BUFFER_SIZE);
                send(connection_fd, "ACK", strlen("ACK")+1, 0);
                if(buffer[0] == '1')
                {
                    recvMessage(connection_fd, buffer, BUFFER_SIZE);
                    send(connection_fd, "ACK", strlen("ACK")+1, 0);

                    printf("%s", buffer);

                    //if player sum is greater than the dealer's
                    recvMessage(connection_fd, buffer, BUFFER_SIZE);
                    send(connection_fd, "ACK", strlen("ACK")+1, 0);
                    if(buffer[0] == '1')
                    {
                        recvMessage(connection_fd, buffer, BUFFER_SIZE);
                        send(connection_fd, "ACK", strlen("ACK")+1, 0);

                        printf("%s", buffer);
                    }else{
                        recvMessage(connection_fd, buffer, BUFFER_SIZE);
                        send(connection_fd, "ACK", strlen("ACK")+1, 0);

                        printf("%s", buffer);
                    }
                }
            }
        }
        recvMessage(connection_fd, buffer, BUFFER_SIZE);
        send(connection_fd, "ACK", strlen("ACK")+1, 0);
    }*/
}

void playerActions(int connection_fd, char * buffer)
{
    char option;
    char busted;

    do{
        bzero(buffer, BUFFER_SIZE);
        recvMessage(connection_fd, buffer, BUFFER_SIZE);//receive menu
        printf("%s", buffer);

        //send option
        scanf("%s", buffer);
        send(connection_fd, buffer, strlen(buffer), 0);

        option = buffer[0];

        if(option == '2')
        {
            getCards(connection_fd, buffer); 

            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            send(connection_fd, "ACK", strlen("ACK")+1, 0);

            busted = buffer[0];
        }
    }while(option == 2 && busted == 0);
}

void communicationLoop(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    
    // Handshake
    send(connection_fd, "START", strlen("START")+1, 0);
    recvMessage(connection_fd, buffer, BUFFER_SIZE);

    //Send available Money to server
    printf("Welcome to Blackjack, type the amount of available Money\n");
    scanf("%s", buffer);
    send(connection_fd, buffer, strlen(buffer), 0);
    recvMessage(connection_fd, buffer, BUFFER_SIZE);

    //Send the bet to the server
    printf("Type your bet\n");
    scanf("%s", buffer);
    send(connection_fd, buffer, strlen(buffer), 0);

    //Receive and show initial cards
    getCards(connection_fd, buffer);

    //if insurance is possible
    insurance(connection_fd, buffer);

    //Check for win communication
    printf("\n");
    checkforwin(connection_fd, buffer);

    //Show available player actions
    //playerActions(connection_fd, buffer);
    
    // Close the socket to the client
    close(connection_fd);
}