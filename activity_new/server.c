#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

typedef struct {
    float bet;
    float insuranceBet;
    char * cards;
    int isBlackjack;
}Player;

typedef struct {
    char * cards;
    int isNatural;
}Dealer;

char isBlackjack(char * cards)
{
    int totalSum = 0;
    int aces_count = 0;

    for(int i=0; i<strlen(cards); i++)
    {
        switch (cards[i])
        {
            case 'A':
                //default value of A as 11
                totalSum += 11;
                aces_count++;
                break;
            case 'J':
                totalSum += 10;
                break;
            case 'Q':
                totalSum += 10;
                break;
            case 'K':
                totalSum += 10;
                break;
            case '0':
                totalSum += 10;
                break;
            default:
                totalSum += cards[i] - 48;
        }
    }

    if(totalSum <= 21)
    {
        return '1';
    }else{
        for(int i=0; i<aces_count; i++)
        {
            if(totalSum <= 21) //try with A=1
            {
                return 1;
            }else{
                totalSum -= 10;
            }
        }

        if(totalSum > 21)
        {
            return '0';
        }
    }
}

char generateCard() {
    char avCards[13] = {'A','2','3','4','5','6','7','8','9','0','J','Q','K'};
    
    int cardIndex = rand()%13;

    return avCards[cardIndex];    
}

void shuffleCards(Player * player, Dealer * dealer)
{
    player->cards = (char *)malloc(21);////////////////////////deallocate when finishing
    dealer->cards = (char *)malloc(21);

    for(int i=0; i<2; i++)
    {
        player->cards[i] = generateCard();
        dealer->cards[i] = generateCard();
    }

    dealer->cards[1] = 'A';

    printf("dealer cards: %s\n", dealer->cards);
    printf("player cards: %s\n", player->cards);
}

void showCards(int connection_fd, char * buffer, int hidden, Player * player, Dealer * dealer)
{
    int chars_read;
    bzero(buffer, BUFFER_SIZE);
    if(hidden == 1)
    {
        //Hidding one card of the dealer's hand
        char * tmp = dealer->cards;
        tmp[0] = '*';

        strcpy(buffer, "Dealer cards: ");
        strcat(buffer, tmp);
        strcat(buffer, "\nPlayer cards: ");
        strcat(buffer, player->cards);
        strcat(buffer, "\n");

        printf("Buffer: \n%s", buffer);
        send(connection_fd, buffer, strlen(buffer), 0);
    }else{
        strcpy(buffer, "Dealer cards: ");
        strcat(buffer, dealer->cards);
        strcat(buffer, "\nPlayer cards: ");
        strcat(buffer, player->cards);
        strcat(buffer, "\n");

        printf("Buffer: \n%s", buffer);
        send(connection_fd, buffer, strlen(buffer), 0);
    }

    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    //printf("ACK showcards: %s\n", buffer);
}

void insurance(int connection_fd, char * buffer, Player * player, Dealer * dealer)
{
    bzero(buffer, BUFFER_SIZE);
    int chars_read;

    if(dealer->cards[1] == 'A' || dealer->cards[1] == '0' || dealer->cards[1] == 'J' || dealer->cards[1] == 'Q' || dealer->cards[1] == 'K')
    {
        strcpy(buffer, "1");
        send(connection_fd, buffer, strlen(buffer), 0);//insurance is possible
        chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);//Receive ACK

        //printf("ACK (insurance possible): %s\n", buffer);

        strcpy(buffer, "Do you want to place an insurance?\n");
        strcat(buffer, "(y) yes / (n) no\n");
        send(connection_fd, buffer, strlen(buffer), 0);//Menu

        chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
        printf("Option received: %s\n", buffer);

        if(buffer == "y")
        {
            strcpy(buffer, "Type your bet:\n");
            send(connection_fd, buffer, strlen(buffer), 0);

            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            sscanf(buffer, "%f", player->insuranceBet);

            printf("Insurance bet received: %f\n", player->insuranceBet);
        }
    }else{
        strcpy(buffer, "0");
        send(connection_fd, buffer, strlen(buffer), 0);
        chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    }
}

void game(int connection_fd, char * buffer)
{
    int chars_read;

    Player player;
    Dealer dealer;

    //Storing bet
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    sscanf(buffer, "%f", &player.bet);
    printf("Received bet: %f\n", player.bet);

    //Shuffling cards
    shuffleCards(&player, &dealer);

    //Showing cards to player
    showCards(connection_fd, buffer, 1, &player, &dealer);

    //if insurance is possible
    insurance(connection_fd, buffer, &player, &dealer);
}

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

    /*//Cards
    char playerCards[21];
    char dealerCards[21];
    int playerCount, dealerCount;
    playerCount = dealerCount = 0;

    //Bet
    float curBet = 0;
    float insuranceBet = 0;

    //flags
    char insurance = '0';
    char naturalFlag = '0';

    //Generate cards for current game
    for(int i=0; i<2; i++)
    {
        playerCards[i] = generateCard();
        dealerCards[i] = generateCard();
    }

    dealerCards[1] = '0';

    printf("Dealer cards: %s\n", dealerCards);
    printf("Player cards: %s\n", playerCards);*/

    // Handshake
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    send(connection_fd, "READY", strlen("READY")+1, 0);

    game(connection_fd, buffer);

    /*//Receieve the bet from the player / client
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    if(chars_read <= 0)
    {
        printf("bet not received, exiting\n");
        close(connection_fd);
        return;
    }

    //Save bet into float variable
    sscanf(buffer, "%f", &curBet);
    printf("Received bet:\t%f\n", curBet);

    //Send initial generated cards to client
    strcpy(buffer, dealerCards);
    buffer[0] = '*'; //Hiding dealer's first card
    send(connection_fd, buffer, strlen(buffer), 0);
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    strcpy(buffer, playerCards);
    send(connection_fd, buffer, strlen(buffer), 0);
    chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);

    //If dealer's face up card is a 10 or A --> Insurance
    if(dealerCards[1] == 'A' || dealerCards[1] == '0')
    {
        insurance = '1';
        //Send result to client
        strcpy(buffer, &insurance);
        send(connection_fd, buffer, strlen(buffer), 0);
        //Receive insurance bet
        chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
        sscanf(buffer, "%f", &insuranceBet);
        printf("Insurance bet received: %f\n", insuranceBet);
    }else{
        //Send result to client
        strcpy(buffer, &insurance);
        send(connection_fd, buffer, strlen(buffer), 0);
        chars_read = recvMessage(connection_fd, buffer, BUFFER_SIZE);
    }


    //Check if dealer has Blackjack
    if(isBlackjack(dealerCards) == 1)
    {

    }*/
    
    
    while(1)
    {
        //Receive the bet from the player
        //sprintf(buffer, "Your cards:\n");

        /*// Get the guess from the user
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
        send(connection_fd, buffer, strlen(buffer)+1, 0);*/
    }
    
    // Close the socket to the client
    close(connection_fd);
}