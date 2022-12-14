#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLENGTH 1023

int main(int argc, char *argv[]) 
{
        int sockfd, port, returnVal;
        char buffer[MAXLENGTH+1], *c;  
        struct sockaddr_in server; //declaring variable



        memset(&server, 0, sizeof(server)); // set server memory

        port = 5556; // set port as first terminal argument
        // (excluding file name)

        sockfd = socket(AF_INET, SOCK_STREAM, 0); // create socket

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(port); // make sure port is in network
        // appropriate format

        if(connect(sockfd, (struct sockaddr*) 
        &server, sizeof(server)) < 0) 
        {
                perror("connection failed.");
                exit(EXIT_FAILURE);
        }
        // connect socket to server address
        
        printf("Connected.\n");

        for(int i = 0; i < 3; i++) 
        {
                                returnVal = read(sockfd, buffer, MAXLENGTH); // read reply
                printf("Read state is %d.\n", returnVal);
                // from socket
                printf("Server : %s", buffer); // print server reply
                printf("Client : "); // print client message
                fgets(buffer, MAXLENGTH, stdin); // get input
                returnVal = write(sockfd, buffer, MAXLENGTH); 
                printf("Write state is %d.\n", returnVal);
                // write message to socket



                if(c=strchr(buffer, '\n')) //check for newline char and clear
                {
                        *c = 0;
                }
        }
        close(sockfd);
}