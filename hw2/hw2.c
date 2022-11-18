/*
 * Server
 *
 * usage: ./hw2 <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sockop.h"

#define REGION_NUM 9
#define STRING_SIZE 100

int sockfd, connfd;

/* Ctrl-C interrupt handler */
void interrupt_handler(int signum) 
{
    close(sockfd);
    close(connfd);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        errexit("Usage: %s <port>\n", argv[0]);
    
    signal(SIGINT, interrupt_handler);

    int command_length;
    int i, j, pow;
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    char snd[BUF_SIZE], rcv[BUF_SIZE];

    unsigned int mild_case[REGION_NUM] = {0};
    unsigned int severe_case[REGION_NUM] = {0};
    unsigned int region_number, case_number, largest_region_number;
    char degree[STRING_SIZE];

    char temp[STRING_SIZE];
    
    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);

    char *p, command[BUF_SIZE][STRING_SIZE];
    while (1) {
        /* Waiting for new connection... */
        printf("Waiting for connection...\n");
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1)       goto err_accept;
        
        /* Connection has been established!! */
        printf("Connection has been established!!\n");

        /* Start using the reporting system */  
        while (1) {
            command_length = 0;

            // Read user commands from the client
            memset(rcv, 0, BUF_SIZE);
            if (read(connfd, rcv, BUF_SIZE) == -1)    goto err_read;

            if (strstr(rcv, "list") != NULL) {
                // Write the menu to the client
                sprintf(snd, "1. Confirmed case\n2. Reporting system\n3. Exit\n");
                if (write(connfd, snd, sizeof(snd)) == -1)    goto err_write;
                continue;
            }

            for (i = 0; i < BUF_SIZE; i++)
                memset(command[i], 0, STRING_SIZE);
            
            // Split the received string w.r.t. '|'    
            p = strtok(rcv, " |\n");
            while (p != NULL) {
                strcpy(command[command_length++], p);
                p = strtok(NULL, " |\n");
            }

            i = 0;
            memset(snd, 0, BUF_SIZE);
            if (strcmp(command[0], "Confirmed") == 0) {
                /* 1. Confirmed case */
                if (command_length == 2) {
                    // List the number of confirmed cases of all regions
                    while (i < REGION_NUM) {
                        sprintf(temp, "%d : %d\n", i, mild_case[i]+severe_case[i]);
                        strcat(snd, temp);
                        i++;
                    }
                    if (write(connfd, snd, sizeof(snd)) == -1)    goto err_write;
                } else if (command_length % 2 == 0) {
                    // List the number of confirmed cases of specific regions
                    while (i < command_length / 2 - 1) {
                        if (strstr(command[(i+1)*2], "Area") == NULL)   goto err_input_confirmed_case;

                        region_number = command[(i+1)*2+1][0] - '0';
                        if (region_number < 0 || region_number >= REGION_NUM)   goto err_outofbound;

                        sprintf(temp, "Area : %d - Mild : %d | Severe : %d\n", region_number, mild_case[region_number], severe_case[region_number]);
                        strcat(snd, temp);
                        i++;     
                    }
                    if (write(connfd, snd, sizeof(snd)) == -1)    goto err_write;
                } else {
                    // User command format error - exit
                    goto err_input_confirmed_case;
                }
                
            } else if (strcmp(command[0], "Reporting") == 0) {
                // 2. Reporting system
                if ((command_length - 2) % 4 == 0) {
                    // Report the case number of specific degree (Mild/Severe) in specific region
                    largest_region_number = 0;
                    while (i < (command_length - 2) / 4) {
                        if (strstr(command[i*4+2], "Area") == NULL)   goto err_input_reporting_system;

                        region_number = command[i*4+3][0] - '0';
                        largest_region_number = (largest_region_number < region_number) ? region_number : largest_region_number;
                        if (region_number < 0 || region_number >= REGION_NUM)   goto err_outofbound;

                        if (strstr(command[(i+1)*4], "Mild") == NULL && strstr(command[(i+1)*4], "Severe") == NULL)   goto err_input_reporting_system;

                        case_number = atoi(command[(i+1)*4+1]);
                        if (strstr(command[(i+1)*4], "Mild") != NULL) {
                            mild_case[region_number] += case_number;
                            sprintf(temp, "Area %d | Mild %d\n", region_number, case_number);
                        } else if (strstr(command[(i+1)*4], "Severe") != NULL) {
                            severe_case[region_number] += case_number;
                            sprintf(temp, "Area %d | Severe %d\n", region_number, case_number);
                        }

                        strcat(snd, temp);
                        i++;
                    }
                    strcpy(temp, "Please wait a few seconds...\n");
                    if (write(connfd, temp, sizeof(temp)) == -1)    goto err_write;

                    sleep(largest_region_number);   // sleep for <largest_region_number> seconds
                    if (write(connfd, snd, sizeof(snd)) == -1)    goto err_write;
                } else {
                    // User command format error - exit
                    goto err_input_reporting_system;
                }

            } else if (strcmp(command[0], "Exit") == 0) {
                // 3. Exit
                break;
            }
            
        }
        
        /* Connection has terminated somehow. */
        printf("Connnection lost.\n\n\n");
        close(connfd);
        
        /* Ready for the next connection... */
    }

    close(sockfd);      /* close server socket */

    return 0;

err_accept:
    fprintf(stderr, "Error: accpet()\n");
    goto err_exit;
err_write:
    fprintf(stderr, "Error: write()\n");
    goto err_exit;
err_read:
    fprintf(stderr, "Error: read()\n");
    goto err_exit;
err_outofbound:
    fprintf(stderr, "Error: array out of bound\n");
    goto err_exit;
err_input_confirmed_case:
    fprintf(stderr, "Usage: Confirmed case [ | Area <region number> | Area <region number> | ...]\n");
    fprintf(stderr, "   each option should be split by an \'|\'\n");
    goto err_exit;
err_input_reporting_system:
    fprintf(stderr, "Usage: Reporting system | Area <region number> | Mild/Severe <case number> | [...]\n");
    fprintf(stderr, "   each option should be split by an \'|\'\n");
    goto err_exit;    

err_exit:
    fprintf(stderr, "Connnection lost. Exiting...\n");
    close(sockfd);
    close(connfd);

    return -1;
}