#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>



#define WRITE "fifoWrite"
#define READ "fifoRead"

void to_uppercase(char *str) {
    while (*str) {
        *str = toupper((unsigned char) *str);
        str++;
    }
}


int main(void)
{

    int fwFifo; // Para escrever as respostas
    int frFifo; // Para ler as strings enviadas pelos clientes
    char str[1024];
    int num_bytes;

    if (access(WRITE, F_OK) == -1)
    {
        if (mkfifo(WRITE, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf(stderr, "Erro ao criar FIFO %s no Servidor\n", WRITE);
            return -1;
        }
    }

    if (access(READ, F_OK) == -1)
    {
        if (mkfifo(READ, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf(stderr, "Erro ao criar FIFO %s no Servidor\n", READ);
            return -1;
        }
    }

    puts("Abrindo FIFO de leitura no Servidor.\n");
    if ((frFifo = open(READ, O_RDONLY | O_NONBLOCK)) < 0)
    {
        fprintf(stderr, "Erro ao abrir a FIFO %s no Servidor\n", READ);
        return -2;
    }

    puts("Abrindo FIFO de escrita no Servidor.\n");
    if ((fwFifo = open(WRITE, O_WRONLY)) < 0)
    {
        fprintf(stderr, "Erro ao abrir a FIFO %s no Servidor\n", WRITE);
        return -2;
    }

    
    puts("Começando a ler...");
    while(1){
        while ((num_bytes = read (frFifo, str, sizeof(str))) > 0)
    {
        // Assegura que a string lida está terminada com '\0'
        str[num_bytes] = '\0';
        puts("Começando a escrever...");
        to_uppercase(str);
        if (write(fwFifo, str, strlen(str)) < 0){
            fprintf(stderr, "Erro ao escrever na FIFO\n");
                return -2;
        };
        puts(str);
        puts("Fim da escrita");
    }
    }


    return 0;
}
