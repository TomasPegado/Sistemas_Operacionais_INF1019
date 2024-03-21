#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 3 // Definindo uma matriz 3x3

// Função para calcular a soma de uma linha
void sum_row(int *A, int *B, int *C, int row) {
    int start = row * SIZE;
    for (int i = 0; i < SIZE; i++) {
        C[start + i] = A[start + i] + B[start + i];
    }
}

int main(void) {
    int shmid_A, shmid_B, shmid_C, status;
    int *A, *B, *C;

    // Alocação de memória compartilhada
    shmid_A = shmget(IPC_PRIVATE, SIZE*SIZE*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    shmid_B = shmget(IPC_PRIVATE, SIZE*SIZE*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    shmid_C = shmget(IPC_PRIVATE, SIZE*SIZE*sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    // Attachment de cada matriz na memória compartilhada
    A = (int *)shmat(shmid_A, NULL, 0);
    B = (int *)shmat(shmid_B, NULL, 0);
    C = (int *)shmat(shmid_C, NULL, 0);

    // Preenchendo as matrizes A e B
    for (int i = 0; i < SIZE*SIZE; i++) {
        A[i] = i + 1; // Exemplo de preenchimento
        B[i] = (i + 1) * 2; // Exemplo de preenchimento
    }

    // Criando processos filhos para calcular cada linha
    for (int i = 0; i < SIZE; i++) {

        if (fork() != 0){ // cria um processo filho para cada linha

            waitpid(-1, &status, 0); // espera cada processo filho terminar
        } else {
            sum_row(A, B, C, i);
            exit(0);
        }
    }


    // Imprimindo a matriz A
    printf("Matriz A:");
    for (int i = 0; i < SIZE*SIZE; i++) {
        if (i % SIZE == 0) printf("\n");
        printf("%d\t", A[i]);
    }
    printf("\n\n");

    // Imprimindo a matriz B
    printf("Matriz B:");
    for (int i = 0; i < SIZE*SIZE; i++) {
        if (i % SIZE == 0) printf("\n");
        printf("%d\t", B[i]);
    }
    printf("\n\n");

    // Imprimindo a matriz resultante
    printf("Matriz Resultante:");
    for (int i = 0; i < SIZE*SIZE; i++) {
        if (i % SIZE == 0) printf("\n");
        printf("%d\t", C[i]);
    }
    printf("\n\n");

    // Fazendo o desattach da memória compartilhada
    shmdt(A);
    shmdt(B);
    shmdt(C);

    // Liberando o local da memória compartilhada
    shmctl(shmid_A, IPC_RMID, NULL);
    shmctl(shmid_B, IPC_RMID, NULL);
    shmctl(shmid_C, IPC_RMID, NULL);

    return 0;
}

