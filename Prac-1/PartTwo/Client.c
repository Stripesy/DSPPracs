#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

// argv[1] = port

#define MAXLENGTH 1024

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[MAXLENGTH];  
    struct sockaddr_in server; 

    memset(&server, 0 ,sizeof(server));

    int port = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons(port);

    char *message;

    int srvLen, msgLen;

    connect(sockfd, (const struct sockaddr*) &server, sizeof(server));
    printf("%s", "Connected.\n");

    for(int i = 0; i < 2; i++) {

    printf("Client : ");

    fgets(message, MAXLENGTH-1, stdin);

    write(sockfd, message, MAXLENGTH);

    int srvLen = read(sockfd, buffer, MAXLENGTH);

    buffer[srvLen] = '\0';

    printf("Server : %s", buffer);

    }

}