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
#include <pthread.h>

#define CLIENT_MAXIMUN 10
#define BUFSIZE 1024
#define errexit(format, arg ...) exit(printf(format, ##arg))

int sockfd, connfd[CLIENT_MAXIMUN];     /* socket descriptor */
unsigned int connfd_occupied[CLIENT_MAXIMUN] = {0};
pid_t childs_pid[CLIENT_MAXIMUN];

/* zombie process handler */
void zombie_process_handler(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/* Ctrl-C interrupt handler */
void interrupt_handler(int signum) 
{
    close(sockfd);
    
    int i;
    for (i = 0; i < CLIENT_MAXIMUN; i++)
        close(connfd[i]);
}

int passivesock(const char *service, const char *transport, int qlen);  /* create server */
void childprocess(int client_number);    /* function for child process */
void *wait_for_child(void *threadid);

/* main function */
int main(int argc, char *argv[])
{
    if (argc != 2)
        errexit("Usage: %s <port>\n", argv[0]);

    signal(SIGCHLD, zombie_process_handler);
    signal(SIGINT, interrupt_handler);

    unsigned short i, client_index = 0;

    pthread_t thread;
    int rc = pthread_create(&thread, NULL, wait_for_child, (void *)0);
    if (rc){
        printf("ERROR; pthread_create() returns %d\n", rc);
        exit(EXIT_FAILURE);
    }

    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);

    struct sockaddr_in addr_cln[CLIENT_MAXIMUN];
    socklen_t sLen = sizeof(addr_cln[0]);

    /* Keep accept client connection */
    while (1) {
        client_index = 0;
        i = 0;
        while (connfd_occupied[i++] == 1) {
            // Find non-occupied connfd
            client_index = i;
        }
        connfd_occupied[client_index] = 1;  // set occupied
        printf("The first available client: %d\n", client_index);

        connfd[client_index] = accept(sockfd, (struct sockaddr *)&addr_cln[client_index], &sLen);
        if (connfd[client_index] == -1)     errexit("Error: accept() of client %d\n", client_index);

        childs_pid[client_index] = fork();
        if (childs_pid[client_index] >= 0) {
            if (childs_pid[client_index] == 0) {
                childprocess(client_index);
            } else {
                printf("Train ID: %d\n", childs_pid[client_index]);
            }
        } else {
            perror("fork");
            exit(0);
        }
    }

    /* close server socket */
    close(sockfd);

    return 0;
}

void childprocess(int client_number)
{    
    dup2(connfd[client_number], STDOUT_FILENO);
    close(connfd[client_number]);

    execlp("sl", "sl", "-l", NULL);

    exit(0);    
}

/* function of thread */
void *wait_for_child(void *threadid) {
    long tid  = (long)threadid;
    int i = 0;
    pid_t pid;

    // Keep tracking the exit of all running child processes
    while (1) {
        for (i = 0; i < CLIENT_MAXIMUN; i++) {
            // Check all running processes
            if (connfd_occupied[i] == 1) {
                pid = waitpid(childs_pid[i], NULL, WNOHANG);
                if (pid == childs_pid[i]) {
                    // child process exit
                    connfd_occupied[i] = 0; // set non-occupied
                }
            }
        }
    }

    pthread_exit(NULL);
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
    
    /* Force using socket address already in use */
    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    
    /* Bind the socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Can't bind to port %s: %s\n", service, strerror(errno));

    /* Set the maximum number of waiting connection */
    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("Can't listen on port %s: %s\n", service, strerror(errno));
    
    return s;
}