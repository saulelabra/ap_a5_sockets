//Saúl Enrique Labra Cruz A01020725
//a5_sockets

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
#define BUFFER_SIZE 500
#define DEFAULT_PORT 8989

// Function declarations
void usage(char * program);
void waitForConnections(int server_fd);
void communicationLoop(int connection_fd);

typedef struct {
    //Money
    float avMoney;
    float bet;
    float winnings;
    float insuranceBet;
    //cards logic
    char * cards;
    int totalSum;
    //flags
    int finsuranceBet;
    int hasBlackjack;
    int stand;
    int busted;
}Player;

typedef struct {
    //cards logic
    char * cards;
    int totalSum;
    //Money
    int winnings;
    //flags
    int isNatural;
    int finished;
}Dealer;

int isBelow21(char * cards, int * totalSum)
{
    *totalSum = 0;
    int aces_count = 0;
    int isBelow = 0;

    for(int i=0; i<strlen(cards); i++)
    {
        switch (cards[i])
        {
            case 'A':
                //default value of A as 11
                *totalSum += 11;
                aces_count++;
                break;
            case 'J':
                *totalSum += 10;
                break;
            case 'Q':
                *totalSum += 10;
                break;
            case 'K':
                *totalSum += 10;
                break;
            case '0':
                *totalSum += 10;
                break;
            default:
                *totalSum += cards[i] - 48;
        }
    }

    if(*totalSum <= 21)
    {
        isBelow = 1;
    }else{
        for(int i=0; i<aces_count; i++)//try with A=1
        {
            if(*totalSum <= 21)
            {
                isBelow = 1;
            }else{
                *totalSum -= 10;
            }
        }

        if(*totalSum > 21)
        {
            isBelow = 0;
        }
    }
    return isBelow;
}

int isNatural(Dealer * dealer)
{
    isBelow21(dealer->cards, &dealer->totalSum);
    //printf("Dealer totalsum = %d\n", dealer->totalSum);
    if(dealer->totalSum == 21)
    {
        return 1;
    }else{
        return 0;
    }
}

int isBlackjack(Player * player)
{
    isBelow21(player->cards, &player->totalSum);
    //printf("Player totalsum = %d\n", player->totalSum);
    if(player->totalSum == 21)
    {
        return 1;
    }else{
        return 0;
    }
}

char generateCard() {
    char avCards[13] = {'A','2','3','4','5','6','7','8','9','0','J','Q','K'};
    
    int cardIndex = rand()%13;

    return avCards[cardIndex];    
}

void initCounters(Player * player, Dealer * dealer)
{
    player->avMoney = 0;
    player->bet = 0;
    player->busted = 0;
    player->finsuranceBet = 0;
    player->hasBlackjack = 0;
    player->insuranceBet = 0;
    player->stand = 0;
    player->totalSum = 0;
    player->winnings = 0;

    dealer->winnings = 0;
    dealer->totalSum = 0;
    dealer->isNatural = 0;
    dealer->finished = 0;
}

void shuffleCards(Player * player, Dealer * dealer)
{
    player->cards = (char *)malloc(21);
    dealer->cards = (char *)malloc(21);

    for(int i=0; i<2; i++)
    {
        player->cards[i] = generateCard();
        dealer->cards[i] = generateCard();
    }

    /*player->cards[0] = '0';
    player->cards[1] = 'A';

    dealer->cards[0] = '0';
    dealer->cards[1] = 'A';*/

    printf("dealer cards: %s\n", dealer->cards);
    printf("player cards: %s\n", player->cards);
}

void showCards(int connection_fd, char * buffer, int hidden, Player * player, Dealer * dealer)
{
    bzero(buffer, BUFFER_SIZE);
    if(hidden == 1)
    {
        //Hidding one card of the dealer's hand
        char tmp[21]; 
        strcpy(tmp, dealer->cards);
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

    recvMessage(connection_fd, buffer, BUFFER_SIZE);
}

void insurance(int connection_fd, char * buffer, Player * player, Dealer * dealer)
{
    bzero(buffer, BUFFER_SIZE);

    if(dealer->cards[1] == 'A' || dealer->cards[1] == '0' || dealer->cards[1] == 'J' || dealer->cards[1] == 'Q' || dealer->cards[1] == 'K')
    {
        strcpy(buffer, "1");
        send(connection_fd, buffer, strlen(buffer), 0);//insurance is possible
        recvMessage(connection_fd, buffer, BUFFER_SIZE);//Receive ACK

        strcpy(buffer, "Do you want to place an insurance?\n");
        strcat(buffer, "(y) yes / (n) no\n");
        send(connection_fd, buffer, strlen(buffer), 0);//Menu

        recvMessage(connection_fd, buffer, BUFFER_SIZE);
        printf("Option received: %s\n", buffer);

        if(buffer[0] == 'y')
        {
            strcpy(buffer, "Type your bet:\n");
            send(connection_fd, buffer, strlen(buffer), 0);

            recvMessage(connection_fd, buffer, BUFFER_SIZE);
            sscanf(buffer, "%f", &player->insuranceBet);
            player->finsuranceBet = 1;

            printf("Insurance bet received: %f\n", player->insuranceBet);
        }
    }else{
        strcpy(buffer, "0");
        send(connection_fd, buffer, strlen(buffer), 0);
        recvMessage(connection_fd, buffer, BUFFER_SIZE);
    }
}

void printOnBoth(int connection_fd, char * buffer)
{
    send(connection_fd, buffer, strlen(buffer)+1, 0);
    recvMessage(connection_fd, buffer, BUFFER_SIZE);
}

int checkforwin(int connection_fd, char *buffer, Player * player, Dealer * dealer)
{
    char outBuffer[BUFFER_SIZE];
    int fisNatural = 0;
    int outFlag = 0;

    if(isNatural(dealer) == 1)//if dealer is natural
    {
        printf("Is natural\n");
        fisNatural = 1;
        sprintf(outBuffer, "The dealer has a natural!\n");

        if(isBlackjack(player) == 1)//if player has Blackjack
        {
            printf("Player has Blackjack\n");
            player->bet = 0;

            sprintf(buffer, "Push (Bet returns to player): The dealer and the player both have Blackjack\nBalance: %f\n", player->avMoney);
            strcat(outBuffer, buffer);

            outFlag = 1;
        }else{
            printf("Player does not have Blackjack\n");
            dealer->winnings += player->bet;
            player->bet = 0;

            sprintf(buffer, "The house wins! Player looses his bet\nBalance: %f\n", player->avMoney);
            strcat(outBuffer, buffer);

            outFlag = 1;
        }

        if(player->finsuranceBet == 1)
        {
            printf("Player has insurance bet\n");

            player->avMoney += 3*player->insuranceBet;
            player->winnings = 2*player->insuranceBet;

            sprintf(buffer, "Player has insurance bet, He gets 2:1 return (%f)\nBalance: %f\n", 2*player->insuranceBet, player->avMoney);
            strcat(outBuffer, buffer);

            player->insuranceBet = 0;

            outFlag = 1;
        }
    }else{
        printf("Dealer is not natural\n");
        if(player->finsuranceBet == 1)
        {
            printf("Player has insurance bet\n");

            sprintf(outBuffer, "Player looses his insurance bet\nBalance: %f\n", player->avMoney);

            dealer->winnings += player->insuranceBet;
            player->insuranceBet = 0;

            outFlag = 1;
        }

        if(isBlackjack(player) == 1)
        {
            printf("Player has Blackjack\n");

            player->avMoney = player->bet;

            sprintf(buffer, "Player wins! He gets 1:1 return (%f)\nBalance: %f\n", player->bet, player->avMoney);
            strcat(outBuffer, buffer);

            player->winnings += player->bet;
            player->bet = 0;

            outFlag = 1;
        }else{
            if(player->stand == 1 && player->busted == 0)
            {
                printf("player has not busted\n");
                if(dealer->finished == 1)
                {
                    printf("Dealer has finished\n");
                    isBelow21(player->cards, &player->totalSum);
                    isBelow21(dealer->cards, &dealer->totalSum);
                    if(player->totalSum > dealer->totalSum)
                    {
                        printf("player sum is bigger\n");
                        player->avMoney += player->bet;
                        sprintf(buffer, "Player wins! He gets 1:1 return (%f)\nBalance: %f\n", player->bet, player->avMoney);
                        strcat(outBuffer, buffer);

                        player->winnings += player->bet;
                        player->bet = 0;

                        outFlag = 1;
                    }else{
                        printf("dealer sum is bigger\n");
                        dealer->winnings += player->bet;
                        player->bet = 0;
                        sprintf(buffer, "The house wins! Player looses his bet\nBalance: %f\n", player->avMoney);
                        strcat(outBuffer, buffer);

                        dealer->winnings += player->bet;
                        player->bet = 0;

                        outFlag = 1;
                    }
                }
            }
        }
    }

    if(outFlag == 1)
    {
        send(connection_fd, "1", strlen("1"), 0);
        recvMessage(connection_fd, buffer, BUFFER_SIZE);

        printOnBoth(connection_fd, outBuffer);

        if(fisNatural == 1)
        {
            send(connection_fd, "1", strlen("1"), 0);
            recvMessage(connection_fd, buffer, BUFFER_SIZE);

            showCards(connection_fd, buffer, 0, player, dealer);
        }else{
            send(connection_fd, "0", strlen("1"), 0);
            recvMessage(connection_fd, buffer, BUFFER_SIZE);
        }
    }else{
        send(connection_fd, "0", strlen("1"), 0);
        recvMessage(connection_fd, buffer, BUFFER_SIZE);
    }
    
    return outFlag;
}

void playerActions(int connection_fd, char * buffer, Player * player, Dealer * dealer)
{
    char option;

    do{
        sprintf(buffer, "Select your option\n(1) Stand\n(2) Hit\n");
        send(connection_fd, buffer, strlen(buffer), 0);//Send menu

        //Receive option
        recvMessage(connection_fd, buffer, BUFFER_SIZE);

        printf("Option received: %s\n", buffer);

        option = buffer[0];

        switch (option)
        {
            case '1':
                player->stand = 1;
                break;
            case '2':
                player->cards[strlen(player->cards)] = generateCard();
                showCards(connection_fd, buffer, 1, player, dealer);
                if(isBelow21(player->cards, &player->totalSum) == 1)
                {
                    player->busted = 0;

                    bzero(buffer, BUFFER_SIZE);
                    send(connection_fd, "0", strlen("0")+1, 0);
                    recvMessage(connection_fd, buffer, BUFFER_SIZE);
                }else{
                    player->busted = 1;

                    bzero(buffer, BUFFER_SIZE);
                    send(connection_fd, "1", strlen("1")+1, 0);
                    recvMessage(connection_fd, buffer, BUFFER_SIZE);
                }
                break;
        }
    }while(option == '2' && player->busted == 0);
}

int isBelow17(char * cards, int * totalSum)
{
    *totalSum = 0;
    int aces_count = 0;
    int isBelow = 0;

    for(int i=0; i<strlen(cards); i++)
    {
        switch (cards[i])
        {
            case 'A':
                //default value of A as 11
                *totalSum += 11;
                aces_count++;
                break;
            case 'J':
                *totalSum += 10;
                break;
            case 'Q':
                *totalSum += 10;
                break;
            case 'K':
                *totalSum += 10;
                break;
            case '0':
                *totalSum += 10;
                break;
            default:
                *totalSum += cards[i] - 48;
        }
    }

    if(*totalSum <= 17)
    {
        isBelow = 1;
    }else{
        for(int i=0; i<aces_count; i++)//try with A=1
        {
            if(*totalSum <= 17)
            {
                isBelow = 1;
            }else{
                *totalSum -= 10;
            }
        }

        if(*totalSum > 17)
        {
            isBelow = 0;
        }
    }
    return isBelow;
}

void dealerContinue(int connection_fd, char * buffer, Player * player, Dealer * dealer)
{
    showCards(connection_fd, buffer, 0, player, dealer);

    do{
        dealer->cards[strlen(dealer->cards)] = generateCard();
        showCards(connection_fd, buffer, 0, player, dealer);

        if(isBelow17(dealer->cards, &dealer->totalSum) == 0)
        {
            dealer->finished = 1;
        }

        buffer[0] = dealer->finished + '0';
        send(connection_fd, buffer, strlen(buffer), 0);
        recvMessage(connection_fd, buffer, BUFFER_SIZE);

    }while(dealer->finished == 0);
    
    return;
}

void freeMem(Player * player, Dealer * dealer)
{
    free(player->cards);
    free(dealer->cards);
}

void game(int connection_fd, char * buffer)
{
    Player player;
    Dealer dealer;

    initCounters(&player, &dealer);

    //Storing available money
    recvMessage(connection_fd, buffer, BUFFER_SIZE);
    sscanf(buffer, "%f", &player.avMoney);
    send(connection_fd, "ACK", strlen("ACK")+1, 0);

    //Storing bet
    recvMessage(connection_fd, buffer, BUFFER_SIZE);
    sscanf(buffer, "%f", &player.bet);
    printf("Received bet: %f\n", player.bet);
    player.avMoney -= player.bet;

    //Shuffling cards
    shuffleCards(&player, &dealer);

    //Showing cards to player
    showCards(connection_fd, buffer, 1, &player, &dealer);

    //if insurance is possible
    insurance(connection_fd, buffer, &player, &dealer);

    //Check for win and continue if posible
    if(checkforwin(connection_fd, buffer, &player, &dealer) == 0)
    {
        //Show options to player
        playerActions(connection_fd, buffer, &player, &dealer);
    }

    //Dealer continue
    if(player.busted == 0)
    {
        dealerContinue(connection_fd, buffer, &player, &dealer);
    }

    //Check for win
    checkforwin(connection_fd, buffer, &player, &dealer);

    //Free memory from cards array
    freeMem(&player, &dealer);
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

    // Handshake
    recvMessage(connection_fd, buffer, BUFFER_SIZE);
    send(connection_fd, "READY", strlen("READY")+1, 0);

    game(connection_fd, buffer);    
    
    // Close the socket to the client
    close(connection_fd);
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