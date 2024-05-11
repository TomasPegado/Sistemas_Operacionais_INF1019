#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

#define MAX_CMD_LEN 1024
#define MAX_PROCESSES 100
#define CYCLE_DURATION 60

typedef struct
{
    char programName[100]; // Nome do programa
    int start;             // Segundo de início no ciclo de 60 segundos
    int duration;          // Duração em segundos
    pid_t pid;             // ID do processo
    int endTime;           // Tempo de término calculado para controle
} RealTimeProcess;

RealTimeProcess rtProcesses[MAX_PROCESSES];
int rtCount = 0;
int rtActive; // Flag para indicar se um processo REAL-TIME está ativo

typedef struct
{
    char programName[100]; // Nome do programa
    int priority;          // Prioridade do processo (1 é a mais alta, 7 é a mais baixa)
    pid_t pid;             // ID do processo Unix do programa em execução
    int active;            // Flag para indicar se o processo está ativo (1) ou pausado (0)
    int finished;          // 0 para não terminado, 1 para terminado
} PriorityProcess;

PriorityProcess prioProcesses[MAX_PROCESSES];
int prioCount = 0; // Número de processos de prioridade carregados
int prioFinished = 0;

typedef struct
{
    char programName[100]; // Nome do programa
    int timeLeft;          // tempo restante na sua fatia
    pid_t pid;
} RoundRobinProcess;

RoundRobinProcess rrProcesses[MAX_PROCESSES];
int rrCount = 0; // Número de processos de round-robin carregados
int currentRRIndex = 0;

int checkConflicts(int start, int duration)
{

    for (int i = 0; i < rtCount; i++)
    {
        int end = rtProcesses[i].start + rtProcesses[i].duration;
        int newEnd = start + duration;
        if ((start >= rtProcesses[i].start && start < end) || (newEnd > rtProcesses[i].start && newEnd <= end))
        {
            return 1; // Conflito encontrado
        }
    }
    return 0; // Sem conflitos
}

void checkAndToggleProcesses(int currentSecond)
{

    for (int i = 0; i < rtCount; i++)
    {
        if (currentSecond == rtProcesses[i].start)
        {
            // Inicia o processo se não estiver em execução
            kill(rtProcesses[i].pid, SIGCONT);
            printf("Processo %s iniciado.\n", rtProcesses[i].programName);
            rtActive = 1; // Marca que um processo REAL-TIME está ativo

            // Pausa todos os processos de prioridade e Round-Robin que possam estar executando
            for (int j = 0; j < prioCount; j++)
            {
                if (prioProcesses[j].active)
                {
                    kill(prioProcesses[j].pid, SIGSTOP);
                    prioProcesses[j].active = 0;
                }
            }
            for (int k = 0; k < rrCount; k++)
            {
                if (rrProcesses[k].timeLeft > 0)
                {
                    kill(rrProcesses[k].pid, SIGSTOP);
                    // rrProcesses[k].timeLeft = 3; // Reset do time slice
                }
            }
        }
        if (currentSecond == rtProcesses[i].endTime)
        {
            // Pausa o processo
            kill(rtProcesses[i].pid, SIGSTOP);
            rtActive = 0;
            printf("Processo %s pausado.\n", rtProcesses[i].programName);
        }
    }

    // Em seguida, verificar processos de prioridade round-robin se nenhum REAL-TIME estiver ativo
    if (!rtActive)
    {
        int highestPriority = 8; // Prioridade vai de 1 a 7, 8 significa sem processo ativo
        int index = -1;
        for (int i = 0; i < prioCount; i++)
        {
            if (!prioProcesses[i].finished && prioProcesses[i].priority < highestPriority)
            {
                highestPriority = prioProcesses[i].priority;
                index = i;
            }
        }

        if (index != -1)
        {
            // Se um processo de maior prioridade é encontrado e é diferente do atualmente ativo
            for (int j = 0; j < prioCount; j++)
            {
                if (prioProcesses[j].active && j != index)
                {
                    kill(prioProcesses[j].pid, SIGSTOP); // Pausa o processo atualmente ativo
                    prioProcesses[j].active = 0;
                    printf("Processo de prioridade %s pausado.\n", prioProcesses[j].programName);
                }
            }

            if (!prioProcesses[index].active)
            {
                kill(prioProcesses[index].pid, SIGCONT); // Inicia o processo de prioridade mais alta
                prioProcesses[index].active = 1;
                printf("Processo de prioridade %s iniciado.\n", prioProcesses[index].programName);
            }
        }
        else if (rrCount > 0)
        { // Se não há processos PRIO para executar, executa Round-Robin
            if (rrProcesses[currentRRIndex].timeLeft > 0)
            {
                kill(rrProcesses[currentRRIndex].pid, SIGCONT);
                rrProcesses[currentRRIndex].timeLeft--;
                printf("Processo de Round-Robin %s iniciado.\n", rrProcesses[currentRRIndex].programName);

                if (rrProcesses[currentRRIndex].timeLeft == 0)
                {
                    kill(rrProcesses[currentRRIndex].pid, SIGSTOP);
                    printf("Processo de Round-Robin %s pausado.\n", rrProcesses[currentRRIndex].programName);
                    currentRRIndex = (currentRRIndex + 1) % rrCount;
                    rrProcesses[currentRRIndex].timeLeft = 3; // Reset do time slice
                }
            }
            else
            {
                // Caso o tempo tenha acabado, troca para o próximo processo
                currentRRIndex = (currentRRIndex + 1) % rrCount;
                rrProcesses[currentRRIndex].timeLeft = 3; // Reset do time slice
            }
        }
    }
}

void sigchld_handler(int sig)
{
    int status;
    pid_t pid;

    // Verifica todos os processos filho que terminaram
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < prioCount; i++)
        {
            if (prioProcesses[i].pid == pid)
            {
                prioProcesses[i].finished = 1; // Marca o processo como terminado
                prioProcesses[i].active = 0;   // Marca o processo como inativo
                prioFinished += 1;
                printf("Processo de prioridade %s finalizado.\n", prioProcesses[i].programName);
                break;
            }
        }
    }
}
// Função para extrair valores de start e duration do RT
void extract_values_RT(const char *buffer, int *start, int *duration)
{
    char *ptr;

    // Zera os valores para indicar falha na extração se necessário
    *start = -1;
    *duration = -1;

    // Encontrar e extrair o valor de start
    ptr = strstr(buffer, "I=");
    if (ptr)
    {
        sscanf(ptr, "I=%d", start);
    }

    // Encontrar e extrair o valor de duration
    ptr = strstr(buffer, "D=");
    if (ptr)
    {
        sscanf(ptr, "D=%d", duration);
    }
}

void extract_values_PRIO(const char *buffer, int *priority)
{

    char *ptr;

    *priority = -1;

    ptr = strstr(buffer, "PR=");
    if (ptr)
    {
        sscanf(ptr, "PR=%d", priority);
    }
}

void termination_handler(int sig)
{
    // Encerra todos os processos REAL-TIME
    for (int i = 0; i < rtCount; i++)
    {
        if (rtProcesses[i].pid != 0)
        { // Verifica se o PID é válido
            kill(rtProcesses[i].pid, SIGTERM);
            printf("Enviando SIGTERM para o processo REAL-TIME %s (PID: %d)\n", rtProcesses[i].programName, rtProcesses[i].pid);
        }
    }

    // Encerra todos os processos Round-Robin
    for (int i = 0; i < rrCount; i++)
    {
        if (rrProcesses[i].pid != 0)
        { // Verifica se o PID é válido
            kill(rrProcesses[i].pid, SIGTERM);
            printf("Enviando SIGTERM para o processo Round-Robin %s (PID: %d)\n", rrProcesses[i].programName, rrProcesses[i].pid);
        }
    }
}

int main()
{
    // Configuração do manipulador de sinal para SIGCHLD
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);
    char buffer[MAX_CMD_LEN];
    char bufferCopy[MAX_CMD_LEN];

    // Configura o manipulador de sinal para SIGTERM usando signal()
    signal(SIGTERM, termination_handler);

    while (fgets(buffer, MAX_CMD_LEN, stdin) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) == 0)
        {             // Verifica se a linha está vazia
            continue; // Pula para a próxima iteração do loop
        }
        strcpy(bufferCopy, buffer); // Faz uma cópia do buffer antes de modificar
        printf("Escalonador recebeu: %s\n", buffer);

        char *programName = strtok(buffer, "-");
        char *schedType = strtok(NULL, " ");
        if (programName == NULL || schedType == NULL)
        {
            fprintf(stderr, "Erro ao extrair informações do programa.\n");
            continue;
        }

        if (strcmp(schedType, "RT") == 0)

        {
            int start, duration;

            // Chamada da função extract_values
            extract_values_RT(bufferCopy, &start, &duration);
            printf("Start: %d\n", start);
            printf("Duration: %d\n", duration);

            // Verifica se os valores foram corretamente atribuídos
            if (start != -1 && duration != -1)
            {
                printf("Programa: %s, Tipo de Escalonamento: %s\n", programName, schedType);
                printf("Start: %d, Duration: %d\n", start, duration);
            }
            else
            {
                printf("Erro ao extrair os dados de tempo.\n");
            }
            if (start + duration > 60)
            {
                fprintf(stderr, "Erro: período de execução vai além de 60 segundos.\n");
                continue;
            }

            if (checkConflicts(start, duration))
            {
                fprintf(stderr, "Conflito encontrado com outro processo REAL-TIME.\n");
                continue;
            }

            // Armazena processo REAL-TIME
            RealTimeProcess newProcess;
            strcpy(newProcess.programName, programName);
            newProcess.start = start;
            newProcess.duration = duration;
            newProcess.endTime = start + duration;
            newProcess.pid = fork();

            if (newProcess.pid == 0)
            {
                char execPath[1024];
                snprintf(execPath, sizeof(execPath), "./%s", programName);
                execlp(execPath, programName, NULL);
                exit(1);
            }
            else
            { // Processo pai

                kill(newProcess.pid, SIGSTOP); // Pausa imediatamente o processo
                rtProcesses[rtCount++] = newProcess;
            }
        }

        else if (strcmp(schedType, "PRIO") == 0)
        {
            int priority;
            int active = 0;

            // Chamada da função extract_values
            extract_values_PRIO(bufferCopy, &priority);

            // Verifica se os valores foram corretamente atribuídos
            if (priority != -1)
            {
                printf("Programa: %s, Tipo de Escalonamento: %s\n", programName, schedType);
                printf("Prioridade: %d, ativo: %d\n", priority, active);
            }
            else
            {
                printf("Erro ao extrair os dados de prioridade.\n");
            }

            // Armazena processo Prioridade
            PriorityProcess newProcess;
            strcpy(newProcess.programName, programName);
            newProcess.priority = priority;
            newProcess.active = active;
            newProcess.finished = 0;
            newProcess.pid = fork();

            if (newProcess.pid == 0)
            {
                char execPath[1024];
                snprintf(execPath, sizeof(execPath), "./%s", programName);
                execlp(execPath, programName, NULL);
                exit(1);
            }
            else
            { // Processo pai

                kill(newProcess.pid, SIGSTOP); // Pausa imediatamente o processo
                prioProcesses[prioCount++] = newProcess;
            }
        }

        else if (strcmp(schedType, "RR") == 0)
        {

            // Armazena processo Prioridade
            RoundRobinProcess newProcess;
            strcpy(newProcess.programName, programName);
            newProcess.timeLeft = 3;
            newProcess.pid = fork();

            if (newProcess.pid == 0)
            {
                char execPath[1024];
                snprintf(execPath, sizeof(execPath), "./%s", programName);
                execlp(execPath, programName, NULL);
                exit(1);
            }
            else
            { // Processo pai

                kill(newProcess.pid, SIGSTOP); // Pausa imediatamente o processo
                rrProcesses[rrCount++] = newProcess;
            }
        }
    }

    int currentSecond = 0;
    int active = 1;
    while (active)
    {
        sleep(1); // Aguarda um segundo
        checkAndToggleProcesses(currentSecond);
        currentSecond = (currentSecond + 1) % CYCLE_DURATION; // Incrementa e reseta o contador a cada 60 segundos
        printf("%d seg\n", currentSecond);
        if (prioFinished == prioCount)
        {
            active = 0;
        }
    }
    printf("Programa finalizando em %d segundos.\n", 60 - currentSecond);
    while ((60 - currentSecond) != 0)
    {
        sleep(1); // Aguarda um segundo
        checkAndToggleProcesses(currentSecond);
        currentSecond = (currentSecond + 1);
        printf("%d seg\n", currentSecond);
    }

    // Enviar SIGTERM ao próprio processo para iniciar a limpeza e finalização
    kill(getpid(), SIGTERM);

    return 0;
}
