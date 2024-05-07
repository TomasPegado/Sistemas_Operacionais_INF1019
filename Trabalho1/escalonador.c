#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CMD_LEN 1024

int main()
{
    char buffer[MAX_CMD_LEN];

    // Lê continuamente do stdin, que está conectado ao pipe
    while (fgets(buffer, MAX_CMD_LEN, stdin) != NULL)
    {
        // Remove a quebra de linha
        buffer[strcspn(buffer, "\n")] = 0;
        printf("Escalonador recebeu: %s\n", buffer);

        // Extrai o nome do programa antes do hífen
        char *programName = strtok(buffer, "-");
        if (programName == NULL)
        {
            fprintf(stderr, "Erro ao extrair o nome do programa.\n");
            continue;
        }

        // Cria um processo filho para executar o programa
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Falha no fork");
            continue;
        }
        else if (pid == 0)
        {
            // Processo filho: executa o programa
            char execPath[1024];
            snprintf(execPath, sizeof(execPath), "./%s", programName); // Monta o caminho relativo
            if (execlp(execPath, programName, (char *)NULL) == -1)
            {
                perror("Falha ao executar o programa");
                exit(1);
            }
        }
        else
        {
            // Processo pai (Escalonador): continua a ler o próximo comando
            printf("Iniciado processo para %s com PID %d\n", programName, pid);
        }
    }

    return 0;
}
