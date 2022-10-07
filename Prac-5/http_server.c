#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<ctype.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdbool.h>

#define BUFFERSIZE 1024

void handle_sigchld(int signalCode);

int main(int argc, char* argv[])
{
        int sockfd, len, connfd, pid, totalSize, it, randomNum, lineCount;
        struct sockaddr_in server, client;
        struct hostent *client_details;
        char buffer[BUFFERSIZE], request[BUFFERSIZE], response[BUFFERSIZE];
        char jacknjill[3000];
        int recvLen;
        int port;
        char* line;
        char* word;
        struct sigaction chldsig;
        char fileLine[3000];
        bool closeCon;
        FILE *file;

        if((file = fopen("jack_jill.txt", "r+")) == NULL)
        {
                fprintf(stderr, "While opening jack_jill.txt");
                exit(EXIT_FAILURE);
        }

        if(argc != 2) 
        {
                fprintf(stderr, "Usage: %s port \n", argv[0]);
                exit(EXIT_FAILURE);
        }
        port = atoi(argv[1]);
        chldsig.sa_handler = handle_sigchld;
        sigfillset(&chldsig.sa_mask);
        chldsig.sa_flags = SA_RESTART | SA_NOCLDSTOP;
        sigaction(SIGCHLD, &chldsig, NULL);

        sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
        {
                perror("While socket()");
                exit(EXIT_FAILURE);
        }
        bzero(&server, sizeof(server));

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(port);

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
        while(1)
        {
                len = sizeof(client);
                bzero(&request, BUFFERSIZE);
                closeCon = false;
                connfd = accept(sockfd, (struct sockaddr *)&client, &len);
                //-exec set follow-fork-mode child
                if(connfd < 0)
                {
                        perror("While accept()");
                        exit(EXIT_FAILURE);
                }
                /*Create a process to handle new client.*/
                if( (pid = fork()) == 0 )
                {
                        srand(time(NULL));
                        randomNum = rand() % 103 + 1;
                        bzero(&jacknjill, 3000);
                        bzero(&fileLine, 3000);
                        while(fgets(fileLine, sizeof(fileLine), file))
                        {
                                strcat(jacknjill, fileLine);
                                lineCount++;
                                if(lineCount == randomNum)
                                {
                                        break;
                                }
                        }
                        /*Child process*/
                        close(sockfd);
                        totalSize = 0;
                        while((recvLen = read(connfd, buffer, sizeof(buffer)-1)) > 0)
                        {       
                                buffer[recvLen] = '\0';

                                for(int i = 0; i < recvLen; i++)
                                {
                                        buffer[i] = toupper(buffer[i]);
                                }
        
                                totalSize += recvLen;
                                if(totalSize >= BUFFERSIZE)
                                {
                                        fprintf(stderr, "Buffer overflow");
                                        exit(EXIT_FAILURE);
                                }
                                strcat(request, "\n");
                                strcat(request, buffer);
                                if(!(buffer[0] == '\r' &&
                                     buffer[1] == '\n'))
                                {
                                        continue;
                                }
                                line = strtok(request, "\r\n");
                                it = 0;
                                bzero(&response, BUFFERSIZE);
                                totalSize = 0;
                                while(line != NULL)
                                {
                                        bzero(&response, sizeof(response));
                                        if(!(strstr(line, "HEAD") == NULL))
                                        {
                                                if(!(strstr(line, " / ") == NULL))
                                                {
                                                        snprintf(response, BUFFERSIZE, 
                                                        "HTTP/1.1 200 OK\r\n"
                                                        "Content-Type: text/plain\r\n"
                                                        "\r\n"); 
                                                        write(connfd, response, sizeof(response));
                                                }
                                                else
                                                {
                                                        snprintf(response, BUFFERSIZE, 
                                                        "HTTP/1.1 404 Not Found\r\n\r\n");
                                                        write(connfd, response, sizeof(response));
                                                        close(connfd);
                                                }
                                        }
                                        else if(!(strstr(line, "GET") == NULL))
                                        {
                                                if(!(strstr(line, " / ") == NULL))
                                                {
                                                        snprintf(response, BUFFERSIZE, 
                                                        "HTTP/1.1 200 OK\r\n"
                                                        "Content-Type: text/plain\r\n"
                                                        "Content-Length: %lu\r\n"
                                                        "\r\n", strlen(jacknjill)); 
                                                        write(connfd, response, sizeof(response));
                                                        write(connfd, jacknjill, sizeof(jacknjill));
                                                }
                                                else
                                                {
                                                
                                                        snprintf(response, BUFFERSIZE, 
                                                        "HTTP/1.1 404 Not Found\r\n\r\n");
                                                        write(connfd, response, sizeof(response));
                                                        close(connfd);
                                                }

                                        }
                                        else if(!(strstr(line, "CONNECTION: CLOSE") == NULL))
                                        {
                                                closeCon = true;
                                        }
                                        else if (it != 0)
                                        {
                                                snprintf(response, BUFFERSIZE, 
                                                "HTTP/1.1 501 Not Implemented\r\n\r\n");
                                        }
                                        line = strtok(NULL, "\r\n");
                                        it++;
                                }
                                if(closeCon)
                                {
                                        close(connfd);
                                        exit(EXIT_SUCCESS);
                                }
                        }
                }
                else
                {
                        /*Parent process*/
                        close(connfd);
                }
        }
        fclose(file);
        close(sockfd);
}

void handle_sigchld(int signalCode) 
{
        pid_t chld;

        while(0 < waitpid(-1, NULL, WNOHANG));
}