#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// argv[1] = port

#define MAXLENGTH 1024

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[MAXLENGTH];  
    struct sockaddr_in server, client;  

    memset(&server, 0 ,sizeof(server));
    memset(&client, 0 ,sizeof(client));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(argc != 2) {
        printf("Incorrect number of arguments. Should be 2, is %d.\n", argc);
        exit(0);
    }

    int port = atoi(argv[1]);
    printf("The set port is %d.\n", port); // Prints the port to error check.

    // Server and client addresses;
    //Set up addresses.
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons(port);

    bind(sockfd, (const struct sockaddr * ) &server, sizeof(server));

    int cliLen, msgLen;

    cliLen = sizeof(client);

    for(int i = 0; i < 2; i++) {

    msgLen = recvfrom(sockfd, buffer, MAXLENGTH, 0, (struct sockaddr *) &client, &cliLen);
    buffer[msgLen] = '\0';
    printf("Client : %s", buffer);

    char *message;
    printf("Server : ");
    fgets(message, MAXLENGTH-1, stdin);

    sendto(sockfd, message, strlen(message), 0,  (const struct sockaddr *) &client, cliLen);
    }

}