#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include<netdb.h>
 
#define BUFFERSIZE 1000
 
int main(int argc, char* argv[])
{
	int sockfd = -1, csd = -1, pid;
        int port = -1;
	struct sockaddr_in6 server_addr, client_addr;
	struct hostent *client_details;
	socklen_t client_addr_len;
	char buffer[BUFFERSIZE];

        // if(argc != 2)
        // {
        //         fprintf(stderr, "Usage: %s port \n", argv[0]);
        //         exit(EXIT_FAILURE);
        // }

	// port = atoi(argv[1]);
	port = 2003;

	if((sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket()");
		return EXIT_FAILURE;
	}
 
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons(port);
 
	if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
	{
		perror("bind()");
		close(sockfd);
		return EXIT_FAILURE;
	}
 
	if (listen(sockfd, 5) < 0) 
	{
		perror("listen()");
		close(sockfd);
		return EXIT_FAILURE;
	}
 
	client_addr_len = sizeof(client_addr);
 
	while(1) 
	{
                bzero(&buffer, BUFFERSIZE);
		csd = accept(sockfd,
				(struct sockaddr*)&client_addr,
				&client_addr_len);
		if (csd == -1) 
		{
			perror("accept()");
			close(sockfd);
			return EXIT_FAILURE;
		}
		client_details = gethostbyaddr((void *) &client_addr.sin6_addr,
                4, AF_INET);
		if(client_details == NULL)
                {
                        herror("Failure to resolve.");
                        exit(EXIT_FAILURE);
                }
		/*Create a process to handle new client.*/
                if( (pid = fork()) == 0 )
                {
                        /*Child process*/
                        close(sockfd);
                        //-exec set follow-fork-mode child
			if (read(csd, &buffer, BUFFERSIZE) < 0) 
			{
			perror("read()");
			close(csd);
			continue;
			}
 
			strcpy(buffer, "Sample response.\n");
			if (write(csd, buffer, BUFFERSIZE) < 0) 
			{
				perror("write()");
				close(csd);
				continue;
			}
	
			if (close(csd) < 0)
			 {
				perror("close()");
				csd = -1;
			}
	
			printf("Connection closed\n");
			{
				exit(EXIT_SUCCESS);
			}
                }
                else
                {
                        /*Parent process*/
                        close(csd);
                        fprintf(stderr, "M: Process %d will service this.\n",
                        pid);
                }
	}
	return EXIT_SUCCESS;
}