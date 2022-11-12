#include <stdio.h>  // perror()
#include <stdlib.h> // exit()
#include <fcntl.h>  // open()
#include <unistd.h> // dup2()

int main(int argc, char *argv[]) {	
	int fd;
	if ((fd = open("test.txt", O_CREAT | O_RDWR, 0644)) == -1) {
		perror("open");
		exit(EXIT_FAILURE);	
	}
	
	dup2(fd, STDOUT_FILENO);
	close (fd);

	printf("Hello, World!\n");
	return 0;
}