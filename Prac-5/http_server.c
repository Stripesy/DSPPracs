#include<stdlib.h>
#include<stdio.h>
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
#define VERBSIZE 10

void handle_sigchld(int signalCode);

int main(int argc, char* argv[])
{
        int sockfd, len, connfd, pid, it;
        struct sockaddr_in server, client;
        struct hostent *client_details;
        char buffer[BUFFERSIZE];
        char response[BUFFERSIZE];
        int recvLen;
        int port = 2013;
        char* word;
        struct sigaction chldsig;
        char verb[VERBSIZE];
        bool connection;
        bool host;

        // if(argc != 2) 
        // {
        //         fprintf(stderr, "Usage: %s port \n", argv[0]);
        //         exit(EXIT_FAILURE);
        // }
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
                        /*Child process*/
                        close(sockfd);
                        while((recvLen = read(connfd, buffer, sizeof(buffer)-1)) > 0)
                        {       
                                it = 0; // 0 = VERB, 1 = DOCUMENT, 2 = VERSION.
                                connection = false;
                                host = false;
                                buffer[recvLen-2] = '\0';
                                word = strtok(buffer, " ");
                                /*

                                ADD CASE INSENSITIVE STRING COMPARISONS

                                */

                                while(word != NULL)
                                {       
                                        switch(it)
                                        {
                                                case 0:
                                                {
                                                        if(sizeof(word) > VERBSIZE)
                                                        {
                                                                perror("Verb Overflow");
                                                                exit(EXIT_FAILURE);
                                                        }
                                                        else if(strcasecmp(word, "HEAD") == 0)
                                                        {
                                                                snprintf(verb,sizeof("HEAD") ,"HEAD");
                                                                /* CHANGE TO ASPRINTF */
                                                        }
                                                        else if(strcasecmp(word, "GET") == 0)
                                                        {
                                                                snprintf(verb,sizeof("GET") ,"GET");
                                                        }
                                                        else if(strcasecmp(word, "Connection:") == 0)
                                                        {
                                                                connection = true;
                                                        }
                                                        else if(strcasecmp(word, "Host:") == 0)
                                                        {
                                                                host = true;
                                                        }
                                                        else
                                                        {
                                                                write(connfd, "501 Not Implemented\n",
                                                                sizeof("501 Not Implemented\n"));
                                                                close(connfd);
                                                                exit(EXIT_SUCCESS);
                                                        }
                                                        break;
                                                }
                                                case 1:
                                                {
                                                        if(((strcasecmp(word, "close") == 0) && connection))
                                                        {
                                                                exit(EXIT_SUCCESS);
                                                        }
                                                        else if(host)
                                                        {
                                                                printf("Do something with host pls");
                                                                exit(EXIT_SUCCESS);
                                                        }
                                                        break;
                                                }
                                                case 2:
                                                {
                                                        break;
                                                }
                                                default:
                                                {

                                                }

                                        } 
                                        word = strtok(NULL, " ");
                                        it++;
                                }
                        }
                }
                else
                {
                        /*Parent process*/
                        close(connfd);
                }
                close(connfd);
        }
        close(sockfd);
}

void handle_sigchld(int signalCode) 
{
        pid_t chld;

        while(0 < waitpid(-1, NULL, WNOHANG));
}