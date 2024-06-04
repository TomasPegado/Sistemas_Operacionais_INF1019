#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define OPENMODE (O_RDONLY | O_NONBLOCK)
#define FIFO "primeiraFifo"

int main(void)
{

    int fpFIFO;
    char ch;
    int n = 15;
    
    puts("Abrindo FIFO");
    if ((fpFIFO = open(FIFO, OPENMODE)) < 0)
    {
        fprintf(stderr, "Erro ao abrir a FIFO %s\n", FIFO);
        return -2;
    }

    while(n){
        sleep(1);
        n--;
    }
    puts("Começando a ler...");
    while (read (fpFIFO, &ch, sizeof(ch)) > 0){
        putchar(ch);
    }
        
    puts("Fim da leitura");
    close(fpFIFO);
    return 0;
}
