#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>


int main(int argc, char *argv[]){

// Receber o parametro indicando a area de memoria

// Fazer o attach de uma variavel int na area

// Fazer o sleep randomico

// Colocar o valor na variavel

// retornar para o pai

    if (argc != 3) {

        printf("Usage: %s <shmid> <i>\n", argv[0]);
        return 1;
    }

    //Convertendo os valores de string para inteiros
    int shmid = atoi(argv[1]);
    int i = atoi(argv[2]);

    //dando attach na memoria
    int *p;
    p = (int *)shmat(shmid, NULL, 0) + i;

    
    //sleep randomico
    srand(time(NULL));
    int sleepTime = rand() % 5 + 1; // numero aleatório entre [1, 10]
    printf("Processo %d ira dormir por %d segundos\n", i+1, sleepTime);
    sleep(sleepTime);
    
    

    //numero randomico
    int randomValue = rand() % 100+1; // Numeros entre 0 e 99
    *p = randomValue;
    printf("Filho %d gerou um novo numero.\n", i+1);


    return 0;

}