#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>

#define NUM_FILHOS 2

int main(void)
{

    int status;
    int fwFifo;
    int frFifo;
    char ch;
    char mensagem[] = "escrita \n";
    if (access("segundaFifo", F_OK) == -1)
    {
        if (mkfifo("segundaFifo", S_IRUSR | S_IWUSR) != 0)
        {
            fprintf(stderr, "Erro ao criar FIFO %s\n", "segundaFifo");
            return -1;
        }
    }

    for (int i = 0; i < NUM_FILHOS; i++)
    {

        if (fork() != 0)
        {

            printf("Abrindo FIFO para escrita no filho %d\n", getpid());
            if ((fwFifo = open("segundaFifo", O_WRONLY | O_NONBLOCK)) < 0)
            {
                fprintf(stderr, "Erro ao abrir a FIFO %s para escrita\n", "segundaFifo");
                return -2;
            }
            puts("Começando a escrever...");
            write(fwFifo, mensagem, strlen(mensagem));
            puts("Fim da escrita");
            close(fwFifo);
            exit(0); // Garantir que o filho termine aqui
        }
        else
        {
            // Processo pai
            if ((frFifo = open("segundaFifo", O_RDONLY | O_NONBLOCK)) < 0)
            {
                fprintf(stderr, "Erro ao abrir a FIFO %s para leitura\n", "segundaFifo");
                return -2;
            };
        }
    }

    // Espera pelos processos filhos terminarem
    for (int i = 0; i < NUM_FILHOS; i++)
    {
        waitpid(-1, &status, 0);
    }

    puts("Começando a ler...");
    while (read(frFifo, &ch, sizeof(ch)) > 0)
        putchar(ch);
    puts("Fim da leitura");

    close(frFifo);
    // Remover a FIFO após o uso
    unlink("segundaFifo");
    return 0;
}