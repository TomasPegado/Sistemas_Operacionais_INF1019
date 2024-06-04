#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{

    int n = 10;
    while (n)
    {
        printf("Executando P1 - Prioridade 7\n");
        sleep(1);
        n--;
    }

    return 0;
}