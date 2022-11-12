#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 10
#define PATH_SIZE 20
int main(int argc, char **argv)
{
    char led0[PATH_SIZE], led1[PATH_SIZE], led2[PATH_SIZE], led3[PATH_SIZE];
    char buf[BUF_SIZE];
    
    snprintf(led0, sizeof(led0), "/dev/LED_0");
    snprintf(led1, sizeof(led1), "/dev/LED_1");
    snprintf(led2, sizeof(led2), "/dev/LED_2");
    snprintf(led3, sizeof(led3), "/dev/LED_3");
    
    int fd0 = open(led0, O_WRONLY);
    if (fd0 < 0) {
	perror("Error opening /dev/LED_0");
	exit(EXIT_FAILURE);
    }
    int fd1 = open(led1, O_WRONLY);
    if (fd1 < 0) {
	perror("Error opening /dev/LED_1");
	exit(EXIT_FAILURE);
    }
    int fd2 = open(led2, O_WRONLY);
    if (fd2 < 0) {
	perror("Error opening /dev/LED_2");
	exit(EXIT_FAILURE);
    }
    int fd3 = open(led3, O_WRONLY);
    if (fd3 < 0) {
	perror("Error opening /dev/LED_3");
	exit(EXIT_FAILURE);
    }

    printf("Set GPIO pins to output.\n");
    int i = 0;
    while (i < 9) {
        //printf("Copying string...\n");
	buf[0] = argv[1][i++];
        buf[1] = '\0';
        
	//printf("Writing...\n");
        if (write(fd0, buf, sizeof(buf)) < 0) {
	    perror("Cannot write fd0");
	    exit(EXIT_FAILURE);
        }
        if (write(fd1, buf, sizeof(buf)) < 0) {
	    perror("Cannot write fd1");
	    exit(EXIT_FAILURE);
        }
        if (write(fd2, buf, sizeof(buf)) < 0) {
	    perror("Cannot write fd2");
	    exit(EXIT_FAILURE);
        }
        if (write(fd3, buf, sizeof(buf)) < 0) {
	    perror("Cannot write fd3");
	    exit(EXIT_FAILURE);
        }
	//printf("Sleeping...\n");
	sleep(1);
    }

    // Set all to 0
    buf[0] = '0';
    buf[1] = '\0';
    if (write(fd0, buf, sizeof(buf)) < 0) {
	perror("Cannot write fd0");
        exit(EXIT_FAILURE);
    }
    if (write(fd1, buf, sizeof(buf)) < 0) {
	perror("Cannot write fd1");
        exit(EXIT_FAILURE);
    }
    if (write(fd2, buf, sizeof(buf)) < 0) {
	perror("Cannot write fd2");
	exit(EXIT_FAILURE);
    }
    if (write(fd3, buf, sizeof(buf)) < 0) {
	perror("Cannot write fd3");
	exit(EXIT_FAILURE);
    }

    printf("Process Complete!!\n");
    return EXIT_SUCCESS;
}

