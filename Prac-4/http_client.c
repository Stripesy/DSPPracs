#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include <unistd.h>

#define BUFFERSIZE 2048

int main(int argc, char* argv[])
{
        int csd;
        char buffer[BUFFERSIZE];
        struct sockaddr_in server_addr; 
        struct hostent *server_host;
        int portNo = 80;
        char reqBuffer[40] = "HEAD / HTTP/1.1\r\n\n";

        if(argc != 2) 
        {
                fprintf(stderr, "Usage: %s website \n", argv[0]);
                exit(EXIT_FAILURE);
        }
        server_host = gethostbyname(argv[1]);
        if(server_host == NULL)
        {
                fprintf(stderr, "Could not find host.\n");
                exit(EXIT_FAILURE);
        }
        csd=socket(PF_INET, SOCK_STREAM, 0);
        if(csd < 0)
        {
                perror("While calling socket()");
                exit(EXIT_FAILURE);
        }
        server_addr.sin_family = AF_INET;
        memcpy(&server_addr.sin_addr.s_addr, server_host->h_addr_list[0],
               server_host->h_length);
                /*Set server address to only accept
               requests from server_host address*/
        server_addr.sin_port = htons(portNo);
        if(connect(csd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        {
                perror("While connect()");
                exit(EXIT_FAILURE);
        }
        printf("Retrieving HTML Head\n");
        if(write(csd, reqBuffer, strlen(reqBuffer)) <= 0) 
        {
                perror("While write()");
                exit(EXIT_FAILURE);
        }
        printf("%s\n",reqBuffer);
        ssize_t recvLen = read(csd, buffer, sizeof(buffer));
        if(recvLen < 0)
        {
                perror("While read()");
                exit(EXIT_FAILURE);
        }
        if(buffer[recvLen-2] != '\r' || buffer[recvLen-1] != '\n')
        {
                printf("Buffer length exceeded.");
                exit(EXIT_FAILURE);
        }
        printf("%s", buffer);
        close(csd);
}