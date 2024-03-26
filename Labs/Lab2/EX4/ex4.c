#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define NUM_FILHOS 2

int main(void)
{

    // Area de memoria compartilhada
    int shmid;
    char stringShmid[20];

    // variaveis
    int *p1, *p2, status;

    // Criar memória compartilhada
    shmid = shmget(IPC_PRIVATE, sizeof(int) * 2, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    sprintf(stringShmid, "%d", shmid);
    p1 = (int *)shmat(shmid, NULL, 0);
    p2 = p1 + 1;

    char num[20];
    for (int i = 0; i < NUM_FILHOS; i++)
    {

        if (fork() != 0)
        {

            waitpid(-1, &status, 0);
        }
        else
        {
            sprintf(num, "%d", i);
            if (execl("./processo", "processo.c", stringShmid, num, NULL) == -1)
            {
                perror("execl failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    printf("p1 = %d\n", *p1);
    printf("p2 = %d\n", *p2);
    printf("produto = %d\n", *p1 * *p2);
    return 0;
}