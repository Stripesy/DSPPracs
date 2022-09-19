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

#define BUFFERSIZE 100000


int main(int argc, char* argv[])
{
        int csd;
        char buffer[BUFFERSIZE];
        struct sockaddr_in server_addr; 
        struct hostent *server_host;
        int portNo = 80;
        char reqBuffer[40];
        char flag[2];

        if(argc != 3) 
        {
                fprintf(stderr, "Usage: %s flag[-g -h] website \n", argv[0]);
                exit(EXIT_FAILURE);
        }
        if(strcmp(argv[1], "-h") == 0) 
        {
                memcpy(&reqBuffer, "HEAD / HTTP/1.1\r\n\n", sizeof("HEAD / HTTP/1.1\r\n\n"));
        }
        else if(strcmp(argv[1], "-g") == 0) 
        {
                memcpy(&reqBuffer, "GET / HTTP/1.1\r\n\n", sizeof("GET / HTTP/1.1\r\n\n"));
        }
        else 
        {
                fprintf(stderr, "Incorrect flag\nUsage: %s flag[-g -h] website\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        server_host = gethostbyname(argv[2]);
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
        printf("Retrieving HTML Reply\n");
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
        if(buffer[9] != '2' || buffer[10] != '0' || buffer[11] != '0') 
        {
                fprintf(stdout ,"Status Code: %c%c%c\n", buffer[9], buffer[10], buffer[11]);
                exit(EXIT_FAILURE);
        }
        if(buffer[recvLen-4] != '\r' || buffer[recvLen-3] != '\n' 
           || buffer[recvLen-2] != '\r' || buffer[recvLen-1] != '\n')
        {
                fprintf(stderr, "Buffer length exceeded.");
                exit(EXIT_FAILURE);
        }
        fprintf(stdout, "%s", buffer);
        close(csd);
}