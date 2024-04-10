#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>


#define EVER ;;

int status;
int shmid;

//  Ponteiros pra guardar PID dos filhos
    int *p1, *p2;

// Criar memória compartilhada
shmid = shmget(IPC_PRIVATE, sizeof(int) * 2, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

p1 = (int *)shmat(shmid, NULL, 0);
p2 = p1 + 1;
int main(void){


    for (int i = 0; i<2; i++){

        if (fork() != 0){

            printf("Processo %d iniciado.\n",i+1);
            
        } else {
            printf("PID do processo: %d.\n", getpid());
            if (i == 0){
                *p1 = getpid();
            } else {
                *p2 = getpid();
            }
            for (EVER);
        }
    }

printf("PID do processo 1: %d.\n", *p1);
printf("PID do processo 2: %d.\n", *p2);
while (waitpid(-1,&status,0)!=-1);



}

// void changeHandles(int sinal)