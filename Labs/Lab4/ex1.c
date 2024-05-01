#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{

    int nDadosTx, nDadosRx; // quantidade de dados transmitidos/recebidos
    int fd[2];              // descritor dos pipes
    const char textoTX[] = "Dados para serem escritos no exercicio 1 do Lab4";
    char textoRX[sizeof textoTX];
    int filho, status;


    if (pipe(fd) < 0)
    {
        puts("Erro ao abrir os pipes");
        exit(-1);
    }

    filho = fork();
    if (filho != 0){

        waitpid(-1,&status,0);
        printf("Filho terminado: %d\n", filho);
        nDadosRx = read(fd[0], textoRX, sizeof textoRX);
        printf("%d dados lidos: %s\n", nDadosRx, textoRX);
        close(fd[0]);

    } else {


        printf("Filho Iniciado: %d\n", getpid());
        nDadosTx = write(fd[1], textoTX, strlen(textoTX) + 1);
        printf("%d dados escritos\n", nDadosTx);
        close(fd[1]);

    }
    
    return 0;
}