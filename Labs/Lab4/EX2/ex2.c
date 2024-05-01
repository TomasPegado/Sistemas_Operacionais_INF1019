#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void)
{
    int pipefd[2];
    pid_t pid;
    char buffer[1024];
    int bytesLidos;

    // Criar o pipe
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Criar o processo filho
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {                     // Processo filho
        close(pipefd[1]); // Fecha o lado de escrita do pipe não usado pelo filho

        // Duplica o lado de leitura do pipe para o descritor de arquivo padrão de entrada (stdin)
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]); // Fecha o pipe original

        // Abre o arquivo de saída
        int outFile = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outFile == -1)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Redireciona a saída padrão para o arquivo de saída
        dup2(outFile, STDOUT_FILENO);
        close(outFile);

        // Lê da entrada padrão (agora ligada ao pipe) e escreve na saída padrão (agora ligada ao arquivo)
        while ((bytesLidos = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0)
        {
            write(STDOUT_FILENO, buffer, bytesLidos);
        }

        exit(EXIT_SUCCESS);
    }
    else
    {                     // Processo pai
        close(pipefd[0]); // Fecha o lado de leitura do pipe não usado pelo pai

        // Abre o arquivo de entrada
        int inFile = open("input.txt", O_RDONLY);
        if (inFile == -1)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Lê do arquivo e escreve no pipe
        while ((bytesLidos = read(inFile, buffer, sizeof(buffer))) > 0)
        {
            write(pipefd[1], buffer, bytesLidos);
        }

        close(inFile);
        close(pipefd[1]); // Fecha o lado de escrita do pipe, indicando o fim dos dados

        // Espera o processo filho terminar
        wait(NULL);
    }

    return 0;
}
