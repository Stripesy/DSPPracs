#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// argv[1] = port

#define MAXLENGTH 1024

int main(int argc, char *argv[])
{
        int sockfd, n;
        char buffer[MAXLENGTH], temp;
        struct sockaddr_in server, client;

        memset(&server, 0, sizeof(server));
        memset(&client, 0, sizeof(client));

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        int port = 1890;
        printf("The set port is %d.\n", port);
        // Prints the port to error check.

        // Server and client addresses;
        // Set up addresses.
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(port);

        bind(sockfd, (const struct sockaddr *)&server, sizeof(server));

        int cliLen, msgLen;

        cliLen = sizeof(client);

        for (int i = 0; i < 2; i++)
        {

                msgLen = recvfrom(sockfd, buffer, MAXLENGTH, 0,
                                  (struct sockaddr *)&client, &cliLen);
                buffer[msgLen] = '\0';
                printf("Client : %s", buffer);

                n = strlen(buffer) - 1;
                for (int i = 0; i < n - 1; i++)
                {
                        for (int j = i + 1; j < n; j++)
                        {
                                if (buffer[i] > buffer[j])
                                {
                                        temp = buffer[i];
                                        buffer[i] = buffer[j];
                                        buffer[j] = temp;
                                }
                        }
                }
                printf("Server : %s", buffer);

                sendto(sockfd, buffer, strlen(buffer), 0,
                       (const struct sockaddr *)&client, cliLen);
        }
}