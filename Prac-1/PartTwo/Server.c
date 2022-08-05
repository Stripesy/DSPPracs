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
        int sockfd, returnVal, port, cliLen, n;
        char buffer[MAXLENGTH], temp;
        struct sockaddr_in server, client;

        if (argc != 2) // check for correct arg count
        {
                printf("Argument count should be 2. Is %d", argc);
                return -1;
        }

        memset(&server, 0, sizeof(server)); // set memory for server addr
        memset(&client, 0, sizeof(client)); // set memory for client addr

        sockfd = socket(AF_INET, SOCK_DGRAM, 0); // create socket

        port = atoi(argv[1]); // set port as first terminal argument
        printf("The set port is %d.\n", port);
        // Prints the port to error check.

        // Server and client addresses;
        // Set up addresses.
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(port);

        bind(sockfd, (const struct sockaddr *)&server, sizeof(server));
        // bind the address to the socket

        cliLen = sizeof(client); // size of client address

        for (int i = 0; i < 2; i++)
        { // loop through 2 its

                returnVal = recvfrom(sockfd, buffer, MAXLENGTH, 0,
                                     (struct sockaddr *)&client, &cliLen);
                printf("Recieve state is %d.\n", returnVal);
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

                returnVal = sendto(sockfd, buffer, strlen(buffer), 0,
                                   (const struct sockaddr *)&client, cliLen);
                printf("Send state is %d.\n", returnVal);
        }
}