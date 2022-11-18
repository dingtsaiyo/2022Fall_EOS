/*
 * Client
 *
 * usage: ./hw2_checker <ip> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sockop.h"

int connfd;

/* Ctrl-C interrupt handler */
void interrupt_handler(int signum) 
{
    close(connfd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
        errexit("Usage: %s <host_address> <host_port>\n", argv[0]);
    
    signal(SIGINT, interrupt_handler);

    char rcv[BUF_SIZE], snd[BUF_SIZE];

    /* create socket and connect to server */
    connfd = connectsock(argv[1], argv[2], "tcp");

    while (1) {
        /* Write user command to the server */
        fgets(snd, sizeof(snd), stdin);
        if (write(connfd, snd, sizeof(snd)) == -1)    goto err_write;
        if (strstr(snd, "Exit") != NULL)   break;

        /* Read the ouput from the server */
        memset(rcv, 0, BUF_SIZE);
        if (read(connfd, rcv, BUF_SIZE) == -1)        goto err_read;
        //printf("Output from server: \n");
        puts(rcv);

        /* Read more information when using reporting system */
        if (strstr(rcv, "Please wait a few seconds...") != NULL) {
            memset(rcv, 0, BUF_SIZE);
            if (read(connfd, rcv, BUF_SIZE) == -1)        goto err_read;
            puts(rcv);
        }

        /* Check if the error has occurred at server */
        if (strstr(rcv, "Error") != NULL) {
            fprintf(stderr, "Some error has occurred. Please check your network connection status or your user command format.\n");
            break;
        }
    }
    
    printf("Disconnect to the server.\n");

    /* close client socket */
    close(connfd);

    return 0;

err_write:
    fprintf(stderr, "Error: write()\n");
    goto err_exit;
err_read:
    fprintf(stderr, "Error: read()\n");
    goto err_exit;

err_exit:
    close(connfd);
    return -1;
}