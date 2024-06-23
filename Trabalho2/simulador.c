#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define NUM_PROCESSOS 3
#define TEMPO_EXECUCAO 3
#define NUM_PAGINAS 16
#define NUM_QUADROS 16

typedef struct
{
    int quadro;
    int presente;
    int referenciado;
    int modificado;
    int contador;
} EntradaTabelaPagina;

typedef struct
{
    EntradaTabelaPagina paginas[NUM_PAGINAS];
} TabelaPagina;

typedef struct
{
    int quadro[NUM_QUADROS];
} MemoriaPrincipal;

MemoriaPrincipal memoria;
TabelaPagina tabelas[NUM_PROCESSOS];

void inicializar_memoria()
{
    for (int i = 0; i < NUM_QUADROS; i++)
    {
        memoria.quadro[i] = -1; // -1 indica que o quadro está livre
    }
}

void inicializar_tabelas()
{
    for (int p = 0; p < NUM_PROCESSOS; p++)
    {
        for (int i = 0; i < NUM_PAGINAS; i++)
        {
            tabelas[p].paginas[i].quadro = -1;
            tabelas[p].paginas[i].presente = 0;
            tabelas[p].paginas[i].referenciado = 0;
            tabelas[p].paginas[i].modificado = 0;
            tabelas[p].paginas[i].contador = 0;
        }
    }
}

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

void iteracao(int *politica, int *num_rodadas, int *parametroWS)
{

    printf("Selecione a política de substituição de páginas:\n");
    printf("1. Not Recently Used (NRU)\n");
    printf("2. Segunda Chance\n");
    printf("3. LRU/Aging\n");
    printf("4. Working Set\n");
    printf("Digite o número da política: ");
    scanf("%d", politica);

    printf("Digite o número de rodadas: ");
    scanf("%d", num_rodadas);

    if (*politica == 1)
    {
        printf("Not Recently Used (NRU)\n");
        printf("Numero de Rodadas: %d\n", *num_rodadas);
    }
    if (*politica == 2)
    {
        printf("Segunda Chance\n");
        printf("Numero de Rodadas: %d\n", *num_rodadas);
    }
    if (*politica == 3)
    {
        printf("LRU/Aging\n");
        printf("\nNumero de Rodadas: %d\n", *num_rodadas);
    }
    if (*politica == 4)
    {
        printf("Informe o parametro: ");
        scanf("%d", parametroWS);
        printf("Working Set (%d)\n", *parametroWS);
        printf("Numero de Rodadas: %d\n", *num_rodadas);
    }
}

int main()
{

    int politica;
    int num_rodadas;
    int parametroWS;

    pid_t pids[NUM_PROCESSOS];
    const char *arquivos[NUM_PROCESSOS] = {"acessos_P1.txt", "acessos_P2.txt", "acessos_P3.txt"};

    inicializar_memoria();
    inicializar_tabelas();

    iteracao(&politica, &num_rodadas, &parametroWS);
    // Cria os processos filhos
    for (int i = 0; i < NUM_PROCESSOS; i++)
    {
        if ((pids[i] = fork()) == 0)
        {
            // Código do processo filho
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
