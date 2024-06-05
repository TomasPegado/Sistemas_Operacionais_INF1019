#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define WRITE "fifoRead"
#define READ "fifoWrite"

int main(void){

    char mensagem[] = "cliente a";
    char resposta[1024];

    int fwFifo; // Para escrever a mensagens
    int frFifo; // Para ler a resposta do servidor
    int n = 5;

    if (access(WRITE, F_OK) == -1)
    {
        if (mkfifo(WRITE, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf(stderr, "Erro ao criar FIFO %s no Cliente A\n", WRITE);
            return -1;
        }
    }

    if (access(READ, F_OK) == -1)
    {
        if (mkfifo(READ, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf(stderr, "Erro ao criar FIFO %s no Cliente A\n", READ);
            return -1;
        }
    }

    puts("Abrindo FIFO de leitura no Cliente A.\n");
    if ((frFifo = open(READ, O_RDONLY | O_NONBLOCK)) < 0)
    {
        fprintf(stderr, "Erro ao abrir a FIFO %s no Cliente A\n", READ);
        return -2;
    }

    puts("Abrindo FIFO de escrita no Cliente A.\n");
    if ((fwFifo = open(WRITE, O_WRONLY)) < 0)
    {
        fprintf(stderr, "Erro ao abrir a FIFO %s no Cliente A\n", WRITE);
        return -2;
    }

    puts("Começando a escrever...");
    write(fwFifo, mensagem, strlen(mensagem));
    puts("Fim da escrita");
    close(fwFifo);

    while(n){
        sleep(1);
        n--;
    }

    puts("Começando a ler...");
    while (read (frFifo, resposta, sizeof(resposta)) > 0){
        puts(resposta);
    }
        
    puts("Fim da leitura");

    return 0;
}
