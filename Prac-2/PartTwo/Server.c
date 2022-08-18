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
#include <sys/wait.h>

#define BUF_LEN 1024

void handle_sigchld(int);
void manage_connection(int, int);
int server_processing(char *instr, char *outstr);

int main(int argc, char *argv[])
{
        int rssd, essd, ec, client_len, pid, port;
        struct sockaddr_in server, client;
        struct hostent *client_details;
        struct sigaction chldsig;

        if(argc != 2)
        {
                printf("Incorrect number of arguments, should be 2, is %d",
                argc);
                exit(EXIT_FAILURE);
        }
        port = atoi(argv[1]);
        fprintf(stderr, "M: The DSAP example server is starting with " \
        "port %d...\n", port);

        chldsig.sa_handler = handle_sigchld;
        sigfillset(&chldsig.sa_mask);
        chldsig.sa_flags = SA_RESTART | SA_NOCLDSTOP;
        sigaction(SIGCHLD, &chldsig, NULL);
        rssd = socket(PF_INET, SOCK_STREAM, 0);
        if( rssd < 0)
        {
                perror("M: While creating the rendezvous socket.");
                exit(EXIT_FAILURE);
        }

        fprintf(stderr, "M:\trssd=%d\n", rssd);
        /* Use INADDR_ANY, this allows the server to receive messages sent to 
        any of its interfaces convert the port number 
        from (h)ost (to) (n)etwork order, there is sometimes a difference.
        */
       memset(&server, 0, sizeof(server));
       server.sin_family = AF_INET;
       server.sin_addr.s_addr = htonl(INADDR_ANY);
       server.sin_port = htons(port);
        /* bind the socket to the server address*/
       if( (ec = bind(rssd, (struct sockaddr * ) &server, sizeof(server))) < 0)
        {
                perror("M: While binding socket");
                exit(EXIT_FAILURE);
        }

        fprintf(stderr, "M:\tbound\n");
        /*Put socket into passive/listen mode to listen for incoming requests*/
        if((ec = listen(rssd, 5)) < 0)
        {
                perror("M: While initializing listen queue");
                exit(EXIT_FAILURE);
        }
        fprintf(stderr, "M:\tlistening\n");
        fprintf(stderr, "M: Setup complete, ready to accept connections.\n");

        while(1)
        {
                /*Main server loop, waits for clients to contact.*/
                client_len = sizeof(client);
                if( (essd = accept(rssd, (struct sockaddr *) &client,
                (socklen_t *) &client_len)) < 0 )
                {
                        perror("M: While accepting a connection.");
                        exit(EXIT_FAILURE);
                }
                client_details = gethostbyaddr((void *) &client.sin_addr.s_addr,
                4, AF_INET);

                if(client_details == NULL)
                {
                        herror("M: While resolving client address");
                        exit(EXIT_FAILURE);
                }
                fprintf(stderr, "M: Accepted a connection from %s on port " \
                "%d with essd=%d\n", client_details->h_name, 
                ntohs(client.sin_port), essd);
                
                /*Create a process to handle new client.*/
                if( (pid = fork()) == 0 )
                {
                        /*Child process*/
                        close(rssd);
                        manage_connection(essd, essd);
                        exit(EXIT_SUCCESS);
                }
                else
                {
                        /*Parent process*/
                        close(essd);
                        fprintf(stderr, "M: Process %d will service this.\n",
                        pid);
                }
        }
        /*Cannot reach here, but may as well close the socket*/
        close(rssd);
}

void manage_connection(int in, int out)
{
        int rc, bc; /*read count, buffer count*/
        char inbuf[BUF_LEN], outbuf[BUF_LEN],
        hostname[40], prefix[100], revbuf[BUF_LEN];

        char end_of_data = '&';
        int i, revcnt;

        gethostname(hostname, 40);
        sprintf(prefix, "\tC %d: ", getpid());
        fprintf(stderr, "\n%sstarting up\n", prefix);
        sprintf(outbuf, "\n\nConnected to 300115 DSAP Example Server on host " \
        "%s\nEnter 'X' as the first character to exit.\nEnter string to be " \
        "case toggled.\n", hostname);

        write(out, outbuf, strlen(outbuf));

        while(1)
        {
                bc = 0;
                while(1)
                {
                        rc = read(in, inbuf,BUF_LEN);
                        if(rc > 0)
                        {
                                if((bc+rc) > BUF_LEN)
                                {
                                        fprintf(stderr, "\n%sRecieve buffer" \
                                        "size exceeded.\n", prefix);
                                        close(in);
                                        exit(EXIT_SUCCESS);
                                }
                                fprintf(stderr, "%sHave read in:\n", prefix);
                                /*dump string*/
                                for(int i = 0; i < rc; i++)
                                {
                                        fprintf(stderr, "%s%d\t%c\n", prefix,
                                        inbuf[i], inbuf[i]);
                                }
                                memcpy(&inbuf[bc], inbuf, rc);
                                bc += rc;

                                /* check if end of buffer */
                                if(inbuf[rc-1] == end_of_data)
                                {
                                        inbuf[rc-1] = '\0';
                                        break;
                                }
                        }
                        else if(rc == 0)
                        {
                                fprintf(stderr, "\n%sClient " \
                                "has closed to connection.\n",
                                prefix);
                                close(in);
                                exit(EXIT_FAILURE);
                        }
                        else
                        {
                                sprintf(prefix, "\tC %d: " \
                                "While reading from " \
                                "connection", getpid());
                                perror(prefix);
                                close(in);
                                exit(EXIT_FAILURE);
                        }
                }
                /*Telnet ends with /r/n so we need to chop
                2 off of the end*/
                //inbuf[bc-2] = '\0';
                if(inbuf[0] == 'X')
                {
                        break;
                }
                revcnt = server_processing(inbuf, revbuf);
                sprintf(outbuf, "The server recieved %d " \
                "characters, which when the case are toggled " \
                "are:\n%s\n\nEnter next string: ", 
                strlen(revbuf), revbuf);
                write(out, outbuf, strlen(outbuf));
        }
        fprintf(stderr, "\n%sClient has exited the session. " \
        "Closing down.\n", prefix);
        close(in);
}

        


int server_processing(char *instr, char *outstr)
{
        int i, len;
        char c;

        len = strlen(instr);

        for(int i = 0; i < len; i++)
        {
                c = tolower(instr[i]);
                if(c == instr[i])
                {
                        outstr[i] = toupper(instr[i]);
                }
                else
                {
                        outstr[i] = c;
                }
        }

        outstr[len] = '\0';

        return len;
}

void handle_sigchld(int signalCode) 
{
        pid_t chld;

        while(0 < waitpid(-1, NULL, WNOHANG));
}