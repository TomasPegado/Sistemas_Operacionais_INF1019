#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int n = 10;
    while (n)
    {
        printf("Executando P2 - Prioridade 2\n");
        sleep(1);
        n--;
    }

    return 0;
}
