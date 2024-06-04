#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NUM_FILHOS 2

int main(void)
{

    int filho;
    int fwFifo;
    int frFifo;
    if (mkfifo("segundaFifo", S_IRUSR | S_IWUSR) == 0)
    {
        puts("FIFO criada com sucesso");
        return 0;
    }
    puts("Erro na criação da FIFO");
    return -1;

    frFifo = open("segundaFifo", O_RDONLY | O_NONBLOCK);
    for (int i = 0; i < NUM_FILHOS; i++)
    {

        if (fork() != 0)
        {

            puts("Abrindo FIFO para escrita no filho %d", getpid);
            if ((fwFifo = open("segundaFifo", O_WRONLY | O_NONBLOCK)) < 0)
            {
                fprintf(stderr, "Erro ao abrir a FIFO %s\n", FIFO);
                return -2;
            }
            puts("Começando a escrever...");
            write(fpFIFO, mensagem, strlen(mensagem));
            puts("Fim da escrita");
            close(fpFIFO);
        }
    }
}