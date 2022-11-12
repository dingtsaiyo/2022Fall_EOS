#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>         // signal()
#include <sys/wait.h>       // waitpid()
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>         // close()

#define CLIENT_NUM 3
#define BUFSIZE 1024
#define errexit(format, arg ...) exit(printf(format, ##arg))

int sockfd, connfd[CLIENT_NUM];     /* socket descriptor */

/* zombie process handler */
void zombie_process_handler(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/* Ctrl-C interrupt handler */
void interrupt_handler(int signum) 
{
    close(sockfd);
}

int passivesock(const char *service, const char *transport, int qlen);  /* create server */
void childprocess(int client_number);    /* function for child process */

/* main function */
int main(int argc, char *argv[])
{
    if (argc != 2)
        errexit("Usage: %s <port>\n", argv[0]);

    signal(SIGCHLD, zombie_process_handler);
    signal(SIGINT, interrupt_handler);

    unsigned short child_index = 0;
    pid_t child_pid[CLIENT_NUM];

    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);

    /* create 3 children */
    while (child_index < CLIENT_NUM) {
        child_pid[child_index] = fork();
        if (child_pid[child_index] >= 0) {
            if (child_pid[child_index] == 0) {
                /* CHILD: accept connection of clients and execute "sl" command */
                childprocess(child_index);
            } else {
                /* PARENT */
                if (child_index == CLIENT_NUM - 1) {
                    /* PARNET: print out the childpid of all children */
                    child_index = 0;
                    while(child_index < CLIENT_NUM)
                        printf("Train ID: %d\n", child_pid[child_index++]);
                } else {
                    /* PARENT: create the next child */
                    child_index++;
                }
            }
        } else {
            errexit("Can't create child %d\n", child_index);
        }
    }

    /* close server socket */
    close(sockfd);

    return 0;
}

void childprocess(int client_number)
{
    struct sockaddr_in addr_cln[CLIENT_NUM];
    socklen_t sLen = sizeof(addr_cln[0]);

    connfd[client_number] = accept(sockfd, (struct sockaddr *)&addr_cln[client_number], &sLen);
    if (connfd[client_number] == -1)
        errexit("Error: accept() of client %d\n", client_number);
    
    dup2(connfd[client_number], STDOUT_FILENO);
    close(connfd[client_number]);

    execlp("sl", "sl", "-l", NULL);

    exit(0);    
}

/*
 * passivesock - allocate & bind a server socket using TCP or UDP
 * 
 * Arguments:
 *  service     - service associated with the desired port
 *  transport   - transport protocol to use ("tcp" or "udp")
 *  qlen        - maximum server request queue length
 */
int passivesock(const char *service, const char *transport, int qlen)
{
    struct servent *pse;        /* pointer to service information entry */
    struct sockaddr_in sin;     /* an Internet endpoint address */
    int s, type;                /* socket descriptor and socket type */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    /* Map service name to port number */
    if ((pse = getservbyname(service, transport)))
        sin.sin_port = htons(ntohs((unsigned short)pse->s_port));
    else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
        errexit("Can't find \"%s\" service entry\n", service);
    
    /* Use protocol to choose a socket type */
    if (strcmp(transport, "udp") == 0) 
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    
    /* Allocate a socket */
    s = socket(PF_INET, type, 0);
    if (s < 0)
        errexit("Can't create socket: %s\n", strerror(errno));
    
    /* Bind the socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Can't bind to port %s: %s\n", service, strerror(errno));

    /* Set the maximum number of waiting connection */
    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("Can't listen on port %s: %s\n", service, strerror(errno));
    
    return s;
}