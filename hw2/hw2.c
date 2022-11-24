/*
 * hw2.c: server of CDC system
 *  [usage] ./hw2 <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sockop.h"

#define REGION_NUM 9
#define STRING_SIZE 100

int sockfd, connfd;
const unsigned short DEBUG = 0;         // debug mode (1:ON | 0:OFF)
const unsigned short OVERWRITE = 1;     // overwrite mode (overwrite previous reporting records)

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

    printf("****************************\n");
    printf("*      DEBUG mode: %s     *\n", DEBUG ? "ON " : "OFF");
    printf("****************************\n");    

    printf("****************************\n");
    printf("*    OVERWRITE mode: %s   *\n", OVERWRITE ? "ON " : "OFF");
    printf("****************************\n");

    int command_length, i, n;
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
        if (connfd == -1)
            goto err_accept;
        
        /* Connection has been established!! */
        printf("Connection has been established!!\n");

        /* Start using the reporting system */  
        while (1) {
            command_length = 0;

            // Read user commands from the client
            memset(rcv, 0, BUF_SIZE);
            if ((n = read(connfd, rcv, BUF_SIZE)) == -1)
                goto err_read;
            
            if (DEBUG)
                printf("Server receives: %s\n", rcv);

            // if (strstr(rcv, "list") != NULL) {
            //     // Write the menu to the client
            //     n = sprintf(snd, "%s", "1. Confirmed case\n2. Reporting system\n3. Exit\n");

            //     if ((n = write(connfd, snd, strlen(snd)+1)) == -1) {
            //         if (DEBUG)  fprintf(stderr, "[list] ");
            //         goto err_write;
            //     }
            //     if (DEBUG)
            //         printf("Server sends: %s\n", snd);
                
            //     continue;
            // }
            
            // Split the received string w.r.t. '|'
            rcv[n] = '\0';
            p = strtok(rcv, " |\n");
            if (DEBUG)  printf("Parsing the string...\n");
            while (p != NULL) {
                if (DEBUG)  printf("    token: %s\n", p);
                strcpy(command[command_length++], p);
                p = strtok(NULL, " |\n");
            }
            if (DEBUG)  printf("    command length = %d\n", command_length);

            i = 0;
            memset(snd, 0, BUF_SIZE);
            if (strcmp(command[0], "list") == 0) {
                /* 0. List the menu */
                n = sprintf(snd, "%s", "1. Confirmed case\n2. Reporting system\n3. Exit\n");

                if ((n = write(connfd, snd, strlen(snd)+1)) == -1) {
                    if (DEBUG)  fprintf(stderr, "[list] ");
                    goto err_write;
                }

                if (DEBUG)  printf("Server sends: %s\n", snd);
            } else if (strcmp(command[0], "Confirmed") == 0) {
                /* 1. Confirmed case */
                if (command_length == 2) {
                    // List the number of confirmed cases of all regions
                    for (i = 0; i < REGION_NUM; i++) {
                        n = sprintf(temp, "%d : %d\n", i, mild_case[i]+severe_case[i]);
                        strcat(snd, temp);
                    }

                    if ((n = write(connfd, snd, strlen(snd)+1)) == -1) {
                        if (DEBUG)  fprintf(stderr, "[Confirmed case (all)] ");
                        goto err_write;
                    }
                    if (DEBUG)  printf("Server sends: %s\n", snd);
                } else if (command_length % 2 == 0) {
                    // List the number of confirmed cases of specific regions
                    for (i = 0; i < command_length / 2 - 1; i++) {
                        if (strstr(command[(i+1)*2], "Area") == NULL)   goto err_input_confirmed_case;

                        // Obtain region number
                        region_number = command[(i+1)*2+1][0] - '0';
                        if (region_number < 0 || region_number >= REGION_NUM)   goto err_outofbound;

                        // Obtain known confirmed cases of the specific region
                        n = sprintf(temp, "Area %d - Mild : %d | Severe : %d\n", region_number, mild_case[region_number], severe_case[region_number]);
                        strcat(snd, temp);
                    }

                    if ((n = write(connfd, snd, strlen(snd)+1)) == -1) {
                        if (DEBUG)  fprintf(stderr, "[Confirmed case (specific)] ");
                        goto err_write;
                    }
                    if (DEBUG)  printf("Server sends: %s\n", snd);
                } else {
                    // User command format error -> exit
                    goto err_input_confirmed_case;
                }
                
            } else if (strcmp(command[0], "Reporting") == 0) {
                // 2. Reporting system
                if ((command_length - 2) % 4 == 0) {
                    // Report the case number of specific degree (Mild/Severe) in specific region
                    largest_region_number = 0;
                    for (i = 0; i < (command_length - 2) / 4; i++) {
                        if (strstr(command[i*4+2], "Area") == NULL)   goto err_input_reporting_system;

                        // Obtain the region of reported confirmed cases
                        region_number = command[i*4+3][0] - '0';
                        largest_region_number = (largest_region_number < region_number) ? region_number : largest_region_number;
                        if (region_number < 0 || region_number >= REGION_NUM)   goto err_outofbound;

                        if (strstr(command[(i+1)*4], "Mild") == NULL && strstr(command[(i+1)*4], "Severe") == NULL)   goto err_input_reporting_system;

                        // Add new cases to the buffer
                        case_number = atoi(command[(i+1)*4+1]);
                        if (strstr(command[(i+1)*4], "Mild") != NULL) {
                            mild_case[region_number] += case_number;
                            n = sprintf(temp, "Area %d | Mild %d\n", region_number, case_number);
                        } else if (strstr(command[(i+1)*4], "Severe") != NULL) {
                            severe_case[region_number] += case_number;
                            n = sprintf(temp, "Area %d | Severe %d\n", region_number, case_number);
                        }

                        strcat(snd, temp);
                    }
                    // waiting messages
                    strcpy(temp, "Please wait a few seconds...\n");
                    if (write(connfd, temp, strlen(temp)+1) == -1) {
                        if (DEBUG)  fprintf(stderr, "[Reporting system (waiting message)] ");
                        goto err_write;
                    }
                    if (DEBUG)  printf("Server sends: %s\n", temp);

                    sleep(largest_region_number);   // sleep for <largest_region_number> seconds
                    // result messages
                    if ((n = write(connfd, snd, strlen(snd)+1)) == -1) {
                        if (DEBUG)  fprintf(stderr, "[Reporting system (result message)] ");
                        goto err_write;
                    }
                    if (DEBUG)  printf("Server sends: %s\n", snd);
                } else {
                    // User command format error -> exit
                    goto err_input_reporting_system;
                }

            } else if (strcmp(command[0], "Exit") == 0) {
                // 3. Exit
                break;
            }

            // Clear the command line buffer
            for (i = 0; i < command_length; i++)
                memset(command[i], 0, STRING_SIZE);            
        }
        
        /* Connection has terminated somehow. */
        printf("Connnection lost.\n\n\n");
        close(connfd);
        
        /* If OVERWRITE = 1 then overwrite the previous records */
        if (OVERWRITE) {
            for (i = 0; i < REGION_NUM; i++) {
                mild_case[i] = 0;
                severe_case[i] = 0;
            }
        }
        
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