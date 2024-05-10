#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{

    int n = 20;
    while (n)
    {
        printf("Executando P3 - Prioridade 1\n");
        sleep(1);
        n--;
    }

    return 0;
}