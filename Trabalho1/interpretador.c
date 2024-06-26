#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main()
{
    FILE *file;
    char line[BUFFER_SIZE];
    int fd[2]; // File descriptors para o pipe

    // Cria o pipe
    if (pipe(fd) == -1)
    {
        perror("Falha ao criar o pipe");
        return 1;
    }

    // Fork para criar o processo escalonador
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Falha no fork");
        return 1;
    }
    else if (pid == 0)
    {
        // Filho: executa o escalonador
        close(fd[1]);              // Fecha o lado de escrita no filho
        dup2(fd[0], STDIN_FILENO); // Redireciona a entrada padrão para ler do pipe
        execlp("./escalonador", "escalonador", NULL);
        perror("Falha ao executar o escalonador");
        exit(1);
    }
    else
    {
        // Código do processo interpretador
        close(fd[0]); // Fecha o lado de leitura do pipe no interpretador
        file = fopen("programas.txt", "r");
        if (file == NULL)
        {
            perror("Falha ao abrir o arquivo");
            return 1;
        }

        while (fgets(line, BUFFER_SIZE, file) != NULL)
        {
            write(fd[1], line, strlen(line)); // Envia linha para o escalonador
            sleep(1);                         // Espera 1 segundo
        }

        fclose(file);
        close(fd[1]); // Fecha o lado de escrita do pipe
        wait(NULL);   // Espera o processo filho terminar
    }

    return 0;
}
