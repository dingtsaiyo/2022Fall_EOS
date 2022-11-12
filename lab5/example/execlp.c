#include <unistd.h>
#include <stdio.h>

void main()
{
    printf("1\n");
    execlp("sl", "sl", "-l", NULL);
    printf("2\n");
}