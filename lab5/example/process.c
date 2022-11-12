/*
* process.c
*/
#include <errno.h>          /* Errors */
#include <stdio.h>          /* Input/Output */
#include <stdlib.h>         /* General Utilities */
#include <sys/types.h>      /* Primitive System Data Types */
#include <sys/wait.h>       /* Wait for Process Termination */
#include <unistd.h>         /* Symbolic Constants */
#include <string.h>

int main(int argc, char *argv[]) {
    int fd1[2]; // Used to store two ends of first pipe
	int fd2[2]; // Used to store two ends of second pipe
    pid_t p;

	if (pipe(fd1) == -1) {
		fprintf(stderr, "Pipe Failed");
		return 1;
	}
	if (pipe(fd2) == -1) {
		fprintf(stderr, "Pipe Failed");
		return 1;
	}

    /* now create new process */
    p = fork();

    char ch;
    char input_str[] = "hello";
    if (p >= 0) { /* fork succeeded */
        if (p == 0) { /* fork() returns 0 to the child process */
            printf("CHILD: I am the child process!\n");
            printf("CHILD: My PID: %d\n", getpid());
            printf("CHILD: My parent's PID is: %d\n", getppid());

            char output_str[100] = "hello";
            while (output_str[0] != 'q') {
                close(fd1[1]);
                read(fd1[0], output_str, 100);
                printf("CHILD: %s\n", output_str);
                sleep(1);
                //close(fd1[0]);
            }

            printf("CHILD: Goodbye!\n");

            exit(0);
        } else { /* fork() returns new pid to the parent process */
            printf("PARENT: I am the parent process!\n");
            printf("PARENT: My PID: %d\n", getpid());
            printf("PARENT: My child's PID is %d\n", p);
            printf("PARENT: I will now wait for my child to exit.\n");

            while (ch != 'q') {
                scanf(" %c", &ch);
                input_str[0] = ch;
                
                close(fd1[0]);
                write(fd1[1], input_str, strlen(input_str) + 1);
                //close(fd1[1]);
                printf("PARENT: %s\n", input_str);
            }

            printf("PARENT: Goodbye!\n");

            exit(0); /* parent exits */
        }
    } else { /* fork returns -1 on failure */
        perror("fork"); /* display error message */
        exit(0);
    }

    return 0;
}