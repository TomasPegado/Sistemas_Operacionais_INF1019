#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define SHM_KEY 8752
#define MAX_SIZE 1024

int main() {
    int shmid, cliente, status;
    char *data;

    // Criar memória compartilhada
    shmid = shmget(SHM_KEY, MAX_SIZE, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // Anexar data a memória compartilhada
    data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)(-1)) {
    perror("shmat");
        exit(1);
    }

    // Ler a mensagem do dia do stdin
    printf("Digite a mensagem do dia: ");
    if (fgets(data, MAX_SIZE, stdin) == NULL) {
        perror("fgets");
        exit(1);
    }

    printf("\n Mensagem salva na memória compartilhada.\n");

    cliente = fork();

    if (cliente != 0){

        waitpid(-1, &status, 0);
        shmdt(data);
        printf("\n Mensagem desanexada da area de memoria.\n");
        shmctl(shmid, IPC_RMID, NULL);
        printf("\n Area de memoria compartilhada identificada por schmid foi removida.\n");
        printf("\n Processos finalizados.\n");
        
    } else {
        printf("\n Processo cliente inicializado.\n");
        printf("\n Mensagem do dia: %s\n", data);
        printf("\n Processo cliente finalizado.\n");

    }
    

    return 0;
}
