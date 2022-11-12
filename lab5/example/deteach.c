/*
* deteach.c -- shows how to detach a thread
*/
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *threadfunc(void *parm)
{
    printf("Thread: Inside secondary thread\n");
    sleep(3);
    printf("Thread: Exit secondary thread\n");
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    pthread_t thread;
    int rc = 0;

    printf("Main: Create a thread using attributes that allow detach\n");
    rc = pthread_create(&thread, NULL, threadfunc, NULL);
    if (rc) {
        printf("ERROR; pthread_create() returns %d\n", rc);
        exit(-1);
    }

    sleep(5);

    printf("Main: Detach the thread after it terminates\n");
    rc = pthread_detach(thread);
    if (rc) {
        printf("ERROR; pthread_detach() returns %d\n", rc);
        exit(-1);
    }

    printf("Main: Detach the thread again\n");
    rc = pthread_detach(thread);
    /* EINVAL: No thread could be found corresponding to that
    * specified by the given thread ID.
    */
    if (rc != EINVAL) {
        printf("Got an unexpected result! rc=%d\n", rc);
        exit(1);
    }

    printf("Main: Second detach fails as expected.\n");
    /* sleep() is not a very robust way to wait for the thread */
    sleep(6);
    printf("Main() completed.\n");
    return 0;
}

