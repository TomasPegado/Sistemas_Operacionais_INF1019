
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

// Função para tratar sinal SIGCONT
void sigcont_handler(int sig) {
    printf("Processo %d continuado com SIGCONT\n", getpid());
}

int main(void) {
    int contador = 0, alternador = 0;
    int shmid;
    int *p1, *p2;

    // Criar memória compartilhada
    shmid = shmget(IPC_PRIVATE, sizeof(int) * 2, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    p1 = (int *)shmat(shmid, NULL, 0);
    p2 = p1 + 1;

    for (int i = 0; i < 2; i++) {
        int pid = fork();
        if (pid > 0) {
            // Processo pai
            printf("Processo %d iniciado.\n", i + 1);
            if (i == 0) *p1 = pid;
            else *p2 = pid;
        } else if (pid == 0) {
            // Processo filho
            printf("PID do processo: %d.\n", getpid());
            
            // Registrar o tratador para SIGCONT
            signal(SIGCONT, sigcont_handler);
            
            while (1); // Loop infinito
        } else {
            // Erro ao criar processo filho
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Dar tempo para que os processos filhos configurem os tratadores de sinal
    sleep(1);

    // Alternar entre os processos filhos usando SIGSTOP e SIGCONT
    while (contador < 10) {
        int pidToStop = alternador == 0 ? *p1 : *p2;
        int pidToCont = alternador == 0 ? *p2 : *p1;
        
        kill(pidToStop, SIGSTOP);
        kill(pidToCont, SIGCONT);
        
        printf("Processo %d pausado, processo %d continuado.\n", pidToStop, pidToCont);
        
        alternador = 1 - alternador;
        contador++;
        sleep(1); // Dar um tempo entre as alternâncias
    }

    // Terminar os processos filhos depois de 10 alternâncias
    kill(*p1, SIGKILL);
    kill(*p2, SIGKILL);
    
    return 0;
}
