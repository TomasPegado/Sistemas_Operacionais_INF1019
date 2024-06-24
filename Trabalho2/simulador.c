#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <limits.h>

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
    unsigned int contador; // Contador para Aging
    unsigned long timestamp;
} EntradaTabelaPagina;

typedef struct
{
    EntradaTabelaPagina paginas[NUM_PAGINAS];
    int fila[NUM_PAGINAS];
    int front;
    int rear;
    int num_elementos;
    int working_set[NUM_PAGINAS];
    int ws_front;
    int ws_rear;
    int ws_count;
} TabelaPagina;

typedef struct
{
    int quadro[NUM_QUADROS];
} MemoriaPrincipal;

typedef struct
{
    int processo;
    int pagina;
    char operacao;
} Mensagem;

MemoriaPrincipal memoria;
TabelaPagina tabelas[NUM_PROCESSOS];
unsigned long global_time = 0;
int page_faults_counter = 0;

void inicializar_memoria()
{
    for (int i = 0; i < NUM_QUADROS; i++)
    {
        memoria.quadro[i] = -1; // -1 indica que o quadro está livre
    }
}

void inicializar_tabelas(int politica)
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
        if (politica == 2)
        {
            tabelas[p].front = 0;
            tabelas[p].rear = -1;
            tabelas[p].num_elementos = 0;
        }
        else if (politica == 3)
        {
            tabelas[p].ws_front = 0;
            tabelas[p].ws_rear = -1;
            tabelas[p].ws_count = 0;
        }
    }
}

// NRU
int notRecentlyUsed(int processo)
{

    int classe[4][NUM_PAGINAS];
    int count[4] = {0, 0, 0, 0};

    // Classifica as páginas em classes
    for (int i = 0; i < NUM_PAGINAS; i++)
    {
        if (tabelas[processo].paginas[i].presente)
        {
            int r = tabelas[processo].paginas[i].referenciado;
            int m = tabelas[processo].paginas[i].modificado;
            int classe_indice = 2 * r + m;
            classe[classe_indice][count[classe_indice]++] = i;
        }
    }

    // Encontra a classe mais baixa não vazia
    for (int i = 0; i < 4; i++)
    {
        if (count[i] > 0)
        {
            int rand_index = rand() % count[i];
            return classe[i][rand_index];
        }
    }

    return -1; // não deve acontecer
}

// Algoritmo Seunda Chance
void adicionar_na_fila(int processo, int pagina)
{
    tabelas[processo].rear = (tabelas[processo].rear + 1) % NUM_PAGINAS;
    tabelas[processo].fila[tabelas[processo].rear] = pagina;
    tabelas[processo].num_elementos++;
}

int segunda_chance(int processo)
{
    while (1)
    {
        int pagina = tabelas[processo].fila[tabelas[processo].front];
        if (tabelas[processo].paginas[pagina].referenciado == 0)
        {
            tabelas[processo].front = (tabelas[processo].front + 1) % NUM_PAGINAS;
            tabelas[processo].num_elementos--;
            return pagina;
        }
        else
        {
            tabelas[processo].paginas[pagina].referenciado = 0;
            tabelas[processo].front = (tabelas[processo].front + 1) % NUM_PAGINAS;
            adicionar_na_fila(processo, pagina);
        }
    }
}

// LRU/Aging
void atualizar_contadores(int processo)
{
    for (int i = 0; i < NUM_PAGINAS; i++)
    {
        if (tabelas[processo].paginas[i].presente)
        {
            tabelas[processo].paginas[i].contador >>= 1;
            if (tabelas[processo].paginas[i].referenciado)
            {
                tabelas[processo].paginas[i].contador |= (1 << (sizeof(unsigned int) * 8 - 1));
                tabelas[processo].paginas[i].referenciado = 0;
            }
        }
    }
}

int lru_aging(int processo)
{
    int pagina_para_substituir = -1;
    unsigned int menor_contador = UINT_MAX;

    for (int i = 0; i < NUM_PAGINAS; i++)
    {
        if (tabelas[processo].paginas[i].presente && tabelas[processo].paginas[i].contador < menor_contador)
        {
            menor_contador = tabelas[processo].paginas[i].contador;
            pagina_para_substituir = i;
        }
    }

    return pagina_para_substituir;
}

// WorkingSet (k)
void atualizar_historico(int processo, int pagina, int k)
{
    // Atualiza o histórico de referências para o working set
    tabelas[processo].paginas[pagina].timestamp = global_time;

    if (tabelas[processo].ws_count < k)
    {
        tabelas[processo].ws_rear = (tabelas[processo].ws_rear + 1) % k;
        tabelas[processo].working_set[tabelas[processo].ws_rear] = pagina;
        tabelas[processo].ws_count++;
    }
    else
    {
        tabelas[processo].ws_front = (tabelas[processo].ws_front + 1) % k;
        tabelas[processo].ws_rear = (tabelas[processo].ws_rear + 1) % k;
        tabelas[processo].working_set[tabelas[processo].ws_rear] = pagina;
    }
}

int working_set(int processo, int k)
{
    int pagina_para_substituir = -1;
    unsigned long oldest_time = ULONG_MAX;

    for (int i = 0; i < NUM_PAGINAS; i++)
    {
        if (tabelas[processo].paginas[i].presente && tabelas[processo].paginas[i].timestamp < oldest_time)
        {
            int found = 0;
            for (int j = 0; j < tabelas[processo].ws_count; j++)
            {
                if (tabelas[processo].working_set[(tabelas[processo].ws_front + j) % k] == i)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                oldest_time = tabelas[processo].paginas[i].timestamp;
                pagina_para_substituir = i;
            }
        }
    }

    return pagina_para_substituir;
}
void gerenciador(int processo, int pagina, char operacao, int politica, int *page_faults, pid_t pid, int k)
{
    global_time++;
    // Envia SIGSTOP para o processo
    kill(pid, SIGSTOP);
    // Atualiza bits de referência e modificação
    tabelas[processo].paginas[pagina].referenciado = 1;
    if (operacao == 'W')
    {
        tabelas[processo].paginas[pagina].modificado = 1;
    }

    if (politica == 4)
    {
        // Atualiza o histórico de referências
        atualizar_historico(processo, pagina, k);
    }

    // Se a página não está presente na memória, deve ser carregada
    if (!tabelas[processo].paginas[pagina].presente)
    {
        printf("Page fault no processo %d, página %d\n", processo, pagina);
        int quadro_vazio = -1;

        // Procura um quadro vazio
        for (int i = 0; i < NUM_QUADROS; i++)
        {
            if (memoria.quadro[i] == -1)
            {
                quadro_vazio = i;
                break;
            }
        }

        // Se não houver quadro vazio, aplica uma politica de substituição
        if (quadro_vazio == -1)

        {

            *page_faults += 1;
            int pagina_para_substituir;
            if (politica == 1)
            { // Aplica NRU
                pagina_para_substituir = notRecentlyUsed(processo);
                quadro_vazio = tabelas[processo].paginas[pagina_para_substituir].quadro;
            }
            else if (politica == 2)
            { // Aplica Segunda Chance
                pagina_para_substituir = segunda_chance(processo);
                quadro_vazio = tabelas[processo].paginas[pagina_para_substituir].quadro;
            }
            else if (politica == 3)
            { // Aplica LRU/Aging

                pagina_para_substituir = lru_aging(processo);
                quadro_vazio = tabelas[processo].paginas[pagina_para_substituir].quadro;
            }
            else if (politica == 4)
            { // Aplica WorkingSet (k)

                pagina_para_substituir = working_set(processo, k);
                quadro_vazio = tabelas[processo].paginas[pagina_para_substituir].quadro;
            }
            else
            {
                perror("Politica Invalida");
                exit(EXIT_FAILURE);
            }

            tabelas[processo].paginas[pagina_para_substituir].presente = 0;
            tabelas[processo].paginas[pagina_para_substituir].quadro = -1;
            printf("Substituindo página %d do processo %d\n", pagina_para_substituir, processo);

            if (tabelas[processo].paginas[pagina_para_substituir].modificado)
            {
                printf("Cópia da página %d do processo %d para o disco de swap\n", pagina_para_substituir, processo);
            }
        }

        // Carrega a nova página
        tabelas[processo].paginas[pagina].quadro = quadro_vazio;
        tabelas[processo].paginas[pagina].presente = 1;
        memoria.quadro[quadro_vazio] = pagina;

        if (politica == 2)
        {
            adicionar_na_fila(processo, pagina);
        }
    }

    printf("Processo %d, Página %d, Operação %c\n", processo, pagina, operacao);

    if (politica == 3)
    {
        // Atualiza os contadores após o gerenciamento da página
        atualizar_contadores(processo);
    }
    // Envia SIGCONT para o processo
    kill(pid, SIGCONT);
}

void executar_processo(const char *nome_arquivo, int processo, int write_fd)
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
        int pagina;
        char operacao;
        sscanf(linha, "%d %c", &pagina, &operacao);
        Mensagem msg = {processo, pagina, operacao};
        write(write_fd, &msg, sizeof(Mensagem));
        sleep(1); // Simular o acesso
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

void imprimir_tabelas_paginas()
{
    for (int p = 0; p < NUM_PROCESSOS; p++)
    {
        printf("Tabela de Páginas do Processo %d:\n", p);
        printf("Página\tQuadro\tPresente\tReferenciado\tModificado\tContador\tTimestamp\n");
        for (int i = 0; i < NUM_PAGINAS; i++)
        {
            printf("%d\t%d\t%d\t\t%d\t\t%d\t\t%u\t\t%lu\n", i, tabelas[p].paginas[i].quadro, tabelas[p].paginas[i].presente,
                   tabelas[p].paginas[i].referenciado, tabelas[p].paginas[i].modificado, tabelas[p].paginas[i].contador, tabelas[p].paginas[i].timestamp);
        }
        printf("\n");
    }
}

void imprimir_memoria()
{
    printf("Estado da Memória Principal:\n");
    printf("Quadro\tPágina\n");
    for (int i = 0; i < NUM_QUADROS; i++)
    {
        if (memoria.quadro[i] == -1)
        {
            printf("%d\tLivre\n", i);
        }
        else
        {
            printf("%d\t%d\n", i, memoria.quadro[i]);
        }
    }
    printf("\n");
}

int main()
{

    int politica;
    int num_rodadas;
    int parametroWS = 0;

    srand(time(NULL));

    int pipes[NUM_PROCESSOS][2];
    pid_t pids[NUM_PROCESSOS];
    const char *arquivos[NUM_PROCESSOS] = {"acessos_P1.txt", "acessos_P2.txt", "acessos_P3.txt"};

    iteracao(&politica, &num_rodadas, &parametroWS);
    inicializar_memoria();
    inicializar_tabelas(politica);

    // Cria os pipes e processos filhos
    for (int i = 0; i < NUM_PROCESSOS; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("Erro ao criar pipe");
            exit(EXIT_FAILURE);
        }

        if ((pids[i] = fork()) == 0)
        {
            // Código do processo filho
            close(pipes[i][0]); // Fecha o lado de leitura do pipe
            executar_processo(arquivos[i], i, pipes[i][1]);
        }
        else if (pids[i] < 0)
        {
            perror("Erro ao criar processo filho");
            exit(EXIT_FAILURE);
        }
        else
        { // Processo pai

            kill(pids[i], SIGSTOP);
            close(pipes[i][1]); // Fecha o lado de escrita do pipe no processo pai
        }
    }

    // Round Robin entre os processos filhos
    int processos_ativos = NUM_PROCESSOS;
    int rodada_atual = 1;
    while (processos_ativos > 0 && rodada_atual <= num_rodadas)
    {
        for (int i = 0; i < processos_ativos; i++)
        {
            if (pids[i] > 0)
            {
                int n = 0;
                while (n < TEMPO_EXECUCAO)
                {
                    // Envia o sinal SIGCONT para continuar a execução do processo filho
                    kill(pids[i], SIGCONT);
                    printf("\nExecutando Processo %d\n", i);
                    // Ler mensagens do pipe
                    Mensagem msg;
                    read(pipes[i][0], &msg, sizeof(Mensagem));
                    gerenciador(msg.processo, msg.pagina, msg.operacao, politica, &page_faults_counter, pids[i], parametroWS);
                    sleep(1);
                    n++;
                }

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
        printf("\nRodada atual: %d\n", rodada_atual);
        rodada_atual++;
    }

    printf("Rodadas finalizadas.\n");
    printf("%d Page Faults.\n", page_faults_counter);

    // Imprime as tabelas de páginas de todos os processos
    imprimir_tabelas_paginas();

    // Imprime o estado da memória principal
    imprimir_memoria();
    return 0;
}
