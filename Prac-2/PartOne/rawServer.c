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
/* port number for the server to use, see note above for setting it */
#define SERVER_TCP_PORT 1890
/* These are used in manage_connection(), the numbers aren’t critical or
 magic they can be set to anything, but BUF_LEN must be greater than
 COM_BUF_LEN
*/
#define BUF_LEN 512
#define COM_BUF_LEN 32
/* prototypes for suport functions */
void manage_connection(int,int);
int server_processing(char *instr, char *outstr);
void handle_sigcld(int);
/* main, no command line args this time! */
int main()
{
int rssd; /* socket descriptor for rendezvous socket */
int essd; /* socket descriptor for empheral socket
 ie. for each connected client */
int ec; /* Error Code holder */
int client_len;
int pid; /* pid of created child */
struct sockaddr_in server,client;
/* address structures for server and
 connected clients */
struct hostent *client_details; /* hostent structure for client name */
struct sigaction cldsig; /* to handle SIGCHLD to prevent zombies
from totally taking over the system */
 fprintf(stderr,"M: The DSAP example Server is starting up...\n");
/* Hand me my boom stick, we are going Zombie hunting! */
/* In order to prevent zombie processes (ie a process whos exit
 status has not been collected) from occuring, each child needs
 to be wait()ed for. This causes a problem as wait() is typically
 a blocking system call. To overcome this, the signal SIGCHLD
 is provided that is sent as a notification that a child process
 has eneded and can be wait()ed on. Here we set up a hanlder
 for SIGCHLD, and ensure no wackyness occurs.
 */
 cldsig.sa_handler=handle_sigcld;
 sigfillset(&cldsig.sa_mask);
 cldsig.sa_flags=SA_RESTART | SA_NOCLDSTOP;
/* zombie destroying!!!! */
 sigaction(SIGCHLD,&cldsig,NULL);
/* We run forever so there is no need to get the old action */
/* create stream orientated rendezvous socket */
 rssd=socket(PF_INET, SOCK_STREAM, 0);
if( rssd < 0 )
 {
 perror("M: While creating the rendezvous socket");
 exit(EXIT_FAILURE);
 }
 fprintf(stderr,"M:\trssd=%d\n",rssd);
/* set up the server address details in order to bind them to a
 specified socket:
 * use the Internet (TCP/IP) address family;
 * use INADDR_ANY, this allows the server to receive messages
 sent to any of its interfaces (Machine IP addresses), this is
 useful for gateway machines and multi−homed hosts;
 * convet the port number from (h)ost (to) (n)etwork order, there
 is sometimes a difference.
 */
 memset(&server,0,sizeof(server));
 server.sin_family=AF_INET;
 server.sin_addr.s_addr=htonl(INADDR_ANY);
 server.sin_port=htons(SERVER_TCP_PORT);
/* bind the socket to the address above */
if( (ec=bind(rssd, (struct sockaddr *) &server, sizeof(server))) < 0 )
 {
 perror("M: While binding the socket");
 exit(EXIT_FAILURE);
 }
 fprintf(stderr,"M:\tbound\n");
/* put the socket in passive mode, so we can get connections
 with accept(). We want to limit ourselves to 5 outstanding
 but this number is not that special.
 */
if((ec=listen(rssd,5))<0)
 {
 perror("M: While seeting the listen queue up");
 exit(EXIT_FAILURE);
 }
 fprintf(stderr,"M:\tlistening\n");
 fprintf(stderr,"M: ... setup complete ready to accept connections\n");
while(1)
 {
/* this is out main server loop, we hang around waiting for
 clients to contact us, and pick them up off the queue
 established by listen() with accept().
 accept() will return a descriptor that can be used to
 communicate with that client, freeing up the socket
 descriptor for new connections.
 */
 client_len=sizeof(client);
if( (essd=accept(rssd, (struct sockaddr *) &client,
 (socklen_t *) &client_len)) < 0 )
 {
 perror("M: While accepting a connetion");
 exit(EXIT_FAILURE);
 }
/* use the resolver to get the name of
 the client so we can print it */
 client_details=gethostbyaddr((void*)&client.sin_addr.s_addr,
 4,AF_INET);
if( client_details == NULL)
 {
 herror("M: While resolving the clients address");
 exit(EXIT_FAILURE);
 }
 fprintf(stderr,"M: Accepted a connection from %s on port %d with essd=%d\n",
 client_details->h_name, ntohs(client.sin_port),
 essd);
/* here we create a process to handle the client */
if( (pid=fork()) == 0)
 {
/* this is the created process, we close the rendezvous
 socket as it isn’t needed here and pass processing
 off to the manage_connection() function.
 */
 close(rssd);
 manage_connection(essd,essd);
/* we’ve handled the client now so we can finish */
 exit(EXIT_SUCCESS);
 }
else
 {
/* the parent simply closes the descriptor from
 accept(), as it doesn’t need it anymore. If
 it wasn’t closed the parent would eventually
 run out of descriptors.
 */
 close(essd);
 fprintf(stderr,"M: process %d will service this.\n",pid);
 }
 }
/* we cant get here but we might as well close the socket anyway */
 close(rssd);
}
void manage_connection(int in, int out)
{
int rc,bc; /* read count and buffer count */
char inbuf[BUF_LEN], outbuf[BUF_LEN], in_data[COM_BUF_LEN],
 hostname[40]; /* buffers for vairous things */
char prefix[100]; /* bit at start of output messages
 to show which child */
char end_of_data='\n';
int i;
int revcnt;
char revbuf[BUF_LEN];
/* get name of machine running the server */
 gethostname(hostname,40);
 sprintf(prefix,"\tC %d: ",getpid());
 fprintf(stderr,"\n%sstarting up\n",prefix);
/* construct the announcment message and send it off */
 sprintf(outbuf,"\n\nConnected to 300115 DSAP Example Server on host: %s\n"\
 "Enter ’X’ as the first character to exit.\nOtherwise enter the "\
 "string to be case toggled.\n",hostname);
 write(out,outbuf,strlen(outbuf));
/*
 Repeatedly call read until we have the enitre message or the
 buffer space runs out. Although TCP guarantees delivery, it
 doesn’t ensure that it arrives as one neat package.
 I use a termination character to tell when end of message is
 reached, but other choices are fixed length messages, or
 prefixing a message with its length.
 Also there is a hard uppper limit on message length, real
 code might push this higher, and allocate memory on the
 fly, but as this is example code I haven’t added this
 complexity.
 */
while(1)
 {
 bc=0;
while(1)
 {
 rc=read(in,in_data,COM_BUF_LEN);
if(rc>0)
 {
if((bc+rc)>BUF_LEN)
 {
 fprintf(stderr,"\n%sReceive buffer size exceeded!\n", prefix);
 close(in);
 exit(EXIT_SUCCESS);
 }
 fprintf(stderr,"%sHave read in:\n",prefix);
/* dump what was read */
for(i=0; i<rc; i++)
 fprintf(stderr,"%s%d\t%c\n",prefix,in_data[i],in_data[i]);
 memcpy(&inbuf[bc], in_data, rc);
 bc=bc+rc;
/* check to see if we have reached the end
 of data marker */
if(in_data[rc-1]==end_of_data) break;
 }
 else if( rc == 0 )
 {
 fprintf(stderr,"\n%sClient has closed the connection so should we.\n",prefix);
 close(in);
 exit(EXIT_FAILURE);
 }
else
 {
 sprintf(prefix,"\tC %d: While reading from connection",getpid());
 perror(prefix);
 close(in);
 exit(EXIT_FAILURE);
 }
 }
/* as telnet ends lines with \r\n we need to chop 2 off the end */
 inbuf[bc-2]='\0';
if(inbuf[0]=='X') break;
/* process the data */
 revcnt=server_processing(inbuf,revbuf);
/* send it back with a message and next prompt */
 sprintf(outbuf,"The server received %d characters, which when the case is toggled"\
 "are:\n%s\n\nEnter next string: ",strlen(revbuf),revbuf);
 write(out,outbuf,strlen(outbuf));
 }
 fprintf(stderr,"\n%sClient has exited the session, closing down\n",prefix);
 close(in);
}
/* quick function to toggle the case of a string */
int server_processing(char *instr, char *outstr)
{
int i, len;
char c;
 len=strlen(instr);
for(i=0;i<len;i++)
 {
 c=tolower(instr[i]);
if(c==instr[i])
 outstr[i]=toupper(instr[i]);
else
 outstr[i]=c;
 }
 outstr[len]='\0';
return len;
}
void handle_sigcld(int signo)
{
pid_t cld;
/* Deal with the potential Zombie horde (Brains... Brains...).
 We dont care about their exit status just that they were
 wait()ed for.*/
while(0<waitpid(-1,NULL,WNOHANG));
/* We need the chainsaw here, as although we get 1 SIGCHLD
 per exited child, we block signals on entering the handler
 so we may miss some (As multiple signal delivery is not
 guaranteed.). This while loop keeps wait()ing until
 all children are accounted for. */
}