#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#define EVER ;;

void killHandler(int sinal);

int main(void) {
    printf("\n pid = %d \n", getpid());
    signal(SIGKILL, killHandler);
    for (EVER);
    return 0;

}


void killHandler(int sinal){

    if (sinal == 9){
        printf("Sinal Kill interceptado (%d)", sinal);
        exit(0);
    }
}