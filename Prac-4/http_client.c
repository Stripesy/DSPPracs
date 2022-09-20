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

#define BUFFERSIZE 8129


int main(int argc, char* argv[])
{
        int csd;
        char buffer[BUFFERSIZE];
        struct sockaddr_in server_addr; 
        struct hostent *server_host;
        int portNo = 80;
        char reqBuffer[BUFFERSIZE];
        char* substring;

        if(argc != 3) 
        {
                fprintf(stderr, "Usage: %s flag[-g -h] website \n", argv[0]);
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
        if(strcmp(argv[1], "-h") == 0) 
        {
        /// Form request
        snprintf(reqBuffer, BUFFERSIZE, 
                "HEAD / HTTP/1.0\r\n" 
                "Host: %s\r\n" 
                "Connection: close\r\n"
                "\r\n", argv[2]);
        }
        else if(strcmp(argv[1], "-g") == 0) 
        {
        /// Form request
        snprintf(reqBuffer, BUFFERSIZE, 
                "GET / HTTP/1.0\r\n" 
                "Host: %s\r\n" 
                "Connection: close\r\n"
                "\r\n", argv[2]);
        }
        else 
        {
                fprintf(stderr, "Incorrect flag\nUsage: %s flag[-g -h] website\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        if(write(csd, reqBuffer, strlen(reqBuffer)) <= 0) 
        {
                perror("While write()");
                exit(EXIT_FAILURE);
        }
        printf("%s\n",reqBuffer);
        ssize_t recvLen = 0;
        int i = 0;
        while((recvLen = read(csd, buffer, sizeof(buffer))) > 0)
        {
                if(strcmp(argv[1], "-g") == 0)
                {
                buffer[recvLen] = '\0';

                printf("%s", buffer);
                i++;
                }
                else if(strcmp(argv[1], "-h") == 0)
                {
                        char* substring;
                        substring = strtok(buffer, "\r\n");
                        while(substring != NULL)
                        {
                                if(strstr(substring, "HTTP/1.0") != NULL)
                                {
                                        printf("%s\n", substring);
                                }
                                else if(strstr(substring, "Content-Type") != NULL)
                                {
                                        printf("%s\n", substring);
                                }
                                else if(strstr(substring, "Last-Modified") != NULL)
                                {
                                        printf("%s\n", substring);
                                }
                                substring = strtok(NULL, "\r\n");
                        }
                }
        }
        if(recvLen < 0)
        {
                perror("While read()");
                exit(EXIT_FAILURE);
        }
        close(csd);
}