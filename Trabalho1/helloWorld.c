#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{

    int n = 10;
    while (n)
    {
        printf("Hello World\n");
        sleep(1);
        n--;
    }

    return 0;
}