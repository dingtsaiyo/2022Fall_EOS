#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sockop.h"

#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    int sockfd, connfd;     /* socket descriptor */
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    int n;
    char snd[BUFSIZE], rcv[BUFSIZE];

    if (argc != 2)
        errexit("Usage: %s <port>\n", argv[0]);
    
    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);

    while (1) {
        /* waiting for connection */
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1)
            errexit("Error: accept()\n");
        
        /* read message from client */
        if ((n = read(connfd, rcv, BUFSIZE)) == -1)
            errexit("Error: read()\n");
        
        printf("n = %d\n", n);
        
        /* write message back to the client */
        n = sprintf(snd, "Server: %.*s", n, rcv);
        printf("n = %d\n", n);
        if ((n = write(connfd, snd, n)) == -1)
            errexit("Error: write()\n");
        printf("n = %d\n", n);
        
        /* close client connection */
        close(connfd);
    }
    /* close server socket */
    close(sockfd);

    return 0;
}