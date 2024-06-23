#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define NUM_PROCESSOS 3
#define TEMPO_EXECUCAO 3

void gerenciador(const char *mensagem)
{
    printf("Gerenciador recebeu: %s\n", mensagem);
}

void executar_processo(const char *nome_arquivo)
{

    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    char linha[20];
    while (fgets(linha, sizeof(linha), arquivo))
    {
        gerenciador(linha);
        sleep(1); // Simula o tempo de processamento de cada acesso
    }

    fclose(arquivo);
    exit(EXIT_SUCCESS);
}

int main()
{
    pid_t pids[NUM_PROCESSOS];
    const char *arquivos[NUM_PROCESSOS] = {"acessos_P1.txt", "acessos_P2.txt", "acessos_P3.txt"};

    // Cria os processos filhos
    for (int i = 0; i < NUM_PROCESSOS; i++)
    {
        if ((pids[i] = fork()) == 0)
        {
            // Código do processo filho
            // kill(pids[i], SIGSTOP);
            executar_processo(arquivos[i]);
        }
        else if (pids[i] < 0)
        {
            perror("Erro ao criar processo filho");
            exit(EXIT_FAILURE);
        }
        else
        { // Processo pai

            kill(pids[i], SIGSTOP);
        }
    }

    // Round Robin entre os processos filhos
    int processos_ativos = NUM_PROCESSOS;
    while (processos_ativos > 0)
    {
        for (int i = 0; i < processos_ativos; i++)
        {
            if (pids[i] > 0)
            {
                // Envia o sinal SIGCONT para continuar a execução do processo filho
                kill(pids[i], SIGCONT);
                printf("\nExecutando Processo %d\n", i);
                sleep(TEMPO_EXECUCAO);
                // Envia o sinal SIGSTOP para parar a execução do processo filho
                kill(pids[i], SIGSTOP);
                printf("\nProcesso %d interrompido\n", i);

                // Verifica se o processo filho terminou
                int status;
                pid_t result = waitpid(pids[i], &status, WNOHANG);
                if (result == pids[i])
                {
                    // Se o processo terminou, espere para limpar seu status
                    waitpid(pids[i], &status, 0);
                    pids[i] = 0; // Processo terminou
                    processos_ativos--;
                }
            }
        }
    }

    printf("Todos os processos filhos terminaram.\n");
    return 0;
}
