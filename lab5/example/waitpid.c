/*
* waitpid.c -- shows how to get child's exit status
*/
#include <errno.h>          /* Errors */
#include <stdio.h>          /* Input/Output */
#include <stdlib.h>         /* General Utilities */
#include <sys/types.h>      /* Primitive System Data Types */
#include <sys/wait.h>       /* Wait for Process Termination */
#include <time.h>           /* Time functions */
#include <unistd.h>         /* Symbolic Constants */

pid_t childpid; /* variable to store the child's pid */
void childfunc(void)
{
    int randtime;           /* random sleep time */
    int exitstatus;         /* random exit status */
    
    printf("CHILD: I am the child process!\n");
    printf("CHILD: My PID: %d\n", getpid());
    
    /* sleep */
    srand(time(NULL));
    randtime = rand() % 5;
    printf("CHILD: Sleeping for %d second...\n", randtime);
    sleep(randtime);

    /* rand exit status */
    exitstatus = rand() % 2;
    printf("CHILD: Exit status is %d\n", exitstatus);

    printf("CHILD: Goodbye!\n");
    exit(exitstatus); /* child exits with user-provided return code */
}


void parentfunc(void)
{
    int status;     /* child's exit status */
    pid_t pid;

    printf("PARENT: I am the parent process!\n");
    printf("PARENT: My PID: %d\n", getpid());

    printf("PARENT: I will now wait for my child to exit.\n");

    /* wait for child to exit, and store its status */
    do {
        pid = waitpid(childpid, &status, WNOHANG);
        printf("PARENT: Waiting child exit ...\n");
        sleep(1);
    } while (pid != childpid);

    if (WIFEXITED(status)) {
        // child process exited normally.
        printf("PARENT: Child's exit code is: %d\n", WEXITSTATUS(status));
    } else {
        // Child process exited thus exec failed.
        // LOG failure of exec in child process.
        printf("PARENT: Child process executed but exited failed.\n");
    }
    printf("PARENT: Goodbye!\n");

    exit(0);        /* parent exits */
}


int main(int argc, char *argv[])
{
    /* now create new process */
    childpid = fork();
    if (childpid >= 0) { /* fork succeeded */
        if (childpid == 0) { /* fork() returns 0 to the child process */
            childfunc();
        } else { /* fork() returns new pid to the parent process */
            parentfunc();
        }
    } else { /* fork returns -1 on failure */
        perror("fork"); /* display error message */
        exit(0);
    }
    return 0;
}