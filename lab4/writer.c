#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 100
#define PATH_SIZE 20
int main(int argc, char **argv)
{
    char mydevice[PATH_SIZE];
    char buf[BUF_SIZE];

	if(argc != 2) {
		printf("Usage: ./writer.o <name>");
		exit(-1);
	}
    
    snprintf(mydevice, sizeof(mydevice), "/dev/mydev");
    
    int fd = open(mydevice, O_WRONLY);
    if (fd < 0) {
        perror("Error opening /dev/mydev");
        exit(EXIT_FAILURE);
    }

    printf("Process Begin!!\n");
    int i = 0;
    ssize_t string_length = strlen(argv[1]);
    while (i < string_length) {
        buf[0] = argv[1][i++];
        buf[1] = '\0';
        
        if (write(fd, buf, sizeof(buf)) < 0) {
            perror("Cannot write fd");
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }

    // print empty character
    buf[0] = '0';
    buf[1] = '\0';
    
    if (write(fd, buf, sizeof(buf)) < 0) {
        perror("Cannot write fd");
        exit(EXIT_FAILURE);
    }

    printf("Process Complete!!\n");
    return EXIT_SUCCESS;
}

