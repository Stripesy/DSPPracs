#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>

#define BUFFERSIZE 1024

int main(int argc, char* argv[])
{
        int sockfd, len, connfd;
        struct sockaddr_in server, client;
        struct hostent *server_host;

        if(argc != 3) 
        {
                fprintf(stderr, "Usage: %s port website \n", argv[0]);
                exit(EXIT_FAILURE);
        }

        server_host = gethostbyname(argv[2]);

        sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
        {
                perror("While socket()");
                exit(EXIT_FAILURE);
        }
        bzero(&server, sizerof(server));

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(argv[1]);

        if((bind(sockfd, (struct sockaddr * )&server, sizeof(server))) != 0)
        {
                perror("While bind()");
                exit(EXIT_FAILURE);
        }

        if((listen(sockfd, 5)) != 0)
        {
                perror("While listen()");
                exit(EXIT_FAILURE);
        }

        len = sizeof(client);

        connfd = accept(sockfd, (struct sockaddr *)&client, &len);
        if(connfd < 0)
        {
                perror("While accept()");
                exit(EXIT_FAILURE);
        }

        close(sockfd);
}