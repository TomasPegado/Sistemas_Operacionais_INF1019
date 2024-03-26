#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define VETOR_TAMANHO 100
#define NUM_PROCESSOS 4

// Função para procurar a chave no vetor
void procurar_chave(int *vetor, int inicio, int fim, int chave, int pid, int *p)
{
    for (int i = inicio; i < fim; i++)
    {
        if (vetor[i] == chave)
        {
            printf("Processo %d encontrou a chave %d na posição %d.\n", pid, chave, i);
            *p = i;
            exit(0); // Termina se encontrar a chave
        }
        // printf("Vetor[%d]: %d.\n", i, vetor[i]);
    }
    printf("Processo %d não encontrou a chave %d.\n", pid, chave);
    exit(0); // Termina se não encontrar a chave
}

int main()
{
    int shmid, status;
    int *vetor, *p;
    int chave = 25; // Exemplo de chave a ser procurada

    // Criar memória compartilhada
    shmid = shmget(IPC_PRIVATE, (VETOR_TAMANHO + 1) * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    p = (int *)shmat(shmid, NULL, 0);
    vetor = p + 1;

    // Preencher o vetor com valores aleatórios
    for (int i = 0; i < VETOR_TAMANHO; i++)
    {
        vetor[i] = rand() % 100; // Valores entre 0 e 99
    }

    int tamanho_parte = VETOR_TAMANHO / NUM_PROCESSOS;

    // Criar processos e dividir o trabalho
    for (int i = 0; i < NUM_PROCESSOS; i++)
    {

        if (fork() != 0)
        {

            waitpid(-1, &status, 0);
        }
        else
        {
            int inicio = i * tamanho_parte;
            int fim = inicio + tamanho_parte;
            procurar_chave(vetor, inicio, fim, chave, getpid(), p);
        }
    }

    printf("\nPosição %d salva no processo pai.\n", *p);

    // Desanexar e remover a memória compartilhada
    shmdt(vetor);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
