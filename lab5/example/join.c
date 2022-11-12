#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 4

void *BusyWork(void *t)
{
    int i;
    long tid;
    double result=0.0;

    tid = (long)t;
    printf("Thread %ld starting...\n",tid);
    for (i = 0; i < 100000000; i++)
    {
        result += 0.0000001;
    }
    
    printf("Thread %ld done. Result = %e\n", tid, result);
    pthread_exit((void*) t);
}

int main (int argc, char *argv[])
{
    pthread_t thread[NUM_THREADS];
    pthread_attr_t attr;
    int rc;
    long t;
    void *status;

    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    printf("A\n");
    for (t = 0; t < NUM_THREADS; t++) {
        printf("Main: creating thread %ld\n", t);
        rc = pthread_create(&thread[t], &attr, BusyWork, (void *)t);
        if (rc) {
            printf("ERROR; pthread_create() returns %d\n", rc);
            exit(-1);
        }
    }

    /* Free attribute and wait for the other threads */
    printf("B\n");
    pthread_attr_destroy(&attr);
    for (t = 0; t < NUM_THREADS; t++) {
        rc = pthread_join(thread[t], &status);
        if (rc) {
            printf("ERROR; pthread_join() returns %d\n", rc);
            exit(-1);
        }
        printf("Main: join with thread %ld (status: %ld)\n", t, (long)status);
    }

    printf("Main: program completed. Exiting.\n");
    pthread_exit(NULL);
}
