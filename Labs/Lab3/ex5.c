#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

int need_new_input = 0;  // Variável global para controle de nova entrada

void zeroHandler(int signal) {
    printf("Divisão por zero é inválido.\n");
    printf("Repita a operação.\n");
    need_new_input = 1;  // Solicita nova entrada
}

void basicOperations(int *v1, int *v2) {
    printf("Insira um número: \n");
    scanf("%d", v1);

    printf("Insira um segundo número: \n");
    scanf("%d", v2);

    printf("%d + %d = %d\n", *v1, *v2, *v1 + *v2);
    printf("%d - %d = %d\n", *v1, *v2, *v1 - *v2);
    printf("%d * %d = %d\n", *v1, *v2, *v1 * *v2);
    
    if (*v2 != 0) {
        printf("%d / %d = %d\n", *v1, *v2, *v1 / *v2);
        need_new_input = 0;
    } else {
        raise(SIGFPE);
    }
}

int main(void) {
    signal(SIGFPE, zeroHandler);
    int v1, v2;
    
    do {
        basicOperations(&v1, &v2);
    } while (need_new_input);  // Continua pedindo novos dados se necessário

    return 0;
}
