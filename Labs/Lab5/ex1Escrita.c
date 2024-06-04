#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define OPENMODE (O_WRONLY | O_NONBLOCK)
#define FIFO "primeiraFifo"

int main(void)
{

    char mensagem[1024];
    scanf("%s", mensagem);


    int fpFIFO;
   
    puts("Abrindo FIFO");
    if ((fpFIFO = open(FIFO, OPENMODE)) < 0)
    {
        fprintf(stderr, "Erro ao abrir a FIFO %s\n", FIFO);
        return -2;
    }
    puts("Começando a escrever...");
    write(fpFIFO, mensagem, strlen(mensagem));
    puts(mensagem);
    puts("Fim da escrita");
    close(fpFIFO);
    return 0;
}