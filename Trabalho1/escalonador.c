#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

#define MAX_CMD_LEN 1024
#define MAX_RT_PROCESSES 100

typedef struct
{
    char programName[100];
    int start;
    int duration;
    pid_t pid;
    timer_t timerid;
} RealTimeProcess;

RealTimeProcess rtProcesses[MAX_RT_PROCESSES];
int rtCount = 0;

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

void sigalrm_handler(int sig, siginfo_t *si, void *uc)
{
    timer_t *tidp = si->si_value.sival_ptr;
    for (int i = 0; i < rtCount; i++)
    {
        if (rtProcesses[i].timerid == *tidp)
        {
            kill(rtProcesses[i].pid, SIGSTOP);
            printf("Processo %s pausado.\n", rtProcesses[i].programName);
            break;
        }
    }
}

int setup_timer(int start, int duration, timer_t *timerid)
{
    struct sigevent sev;
    struct itimerspec its;
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigalrm_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = timerid;
    if (timer_create(CLOCK_REALTIME, &sev, timerid) == -1)
    {
        perror("timer_create");
        return -1;
    }

    its.it_value.tv_sec = start + duration;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    if (timer_settime(*timerid, 0, &its, NULL) == -1)
    {
        perror("timer_settime");
        return -1;
    }

    return 0;
}

void scheduleRealTimeProcesses()
{
    while (1)
    {
        time_t currentTime = time(NULL);
        struct tm *timeinfo = localtime(&currentTime);
        int currentSecond = timeinfo->tm_sec;

        for (int i = 0; i < rtCount; i++)
        {
            // Se o segundo atual corresponde ao início do processo, inicie ou continue o processo
            if (currentSecond == rtProcesses[i].start)
            {
                kill(rtProcesses[i].pid, SIGCONT);
                printf("Processo %s iniciado ou continuado.\n", rtProcesses[i].programName);

                // Agende a parada do processo após a duração especificada
                struct itimerspec its;
                its.it_value.tv_sec = rtProcesses[i].duration;
                its.it_value.tv_nsec = 0;
                its.it_interval.tv_sec = 0; // Não repetir automaticamente
                its.it_interval.tv_nsec = 0;
                timer_settime(rtProcesses[i].timerid, 0, &its, NULL);
            }
        }

        // Espera até o próximo segundo para verificar novamente
        struct timespec ts = {.tv_sec = 1, .tv_nsec = 0};
        nanosleep(&ts, NULL);
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

int main()
{
    char buffer[MAX_CMD_LEN];
    char bufferCopy[MAX_CMD_LEN];

    // Exemplo de configuração de um timer para um processo
    if (rtCount < MAX_RT_PROCESSES)
    {
        // Configura e armazena informações do processo
        setup_timer(rtProcesses[rtCount].start, rtProcesses[rtCount].duration, &rtProcesses[rtCount].timerid);
        rtCount++;
    }

    while (fgets(buffer, MAX_CMD_LEN, stdin) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = 0;
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
            newProcess.pid = fork();

            if (newProcess.pid == 0)
            {
                char execPath[1024];
                snprintf(execPath, sizeof(execPath), "./%s", programName);
                execlp(execPath, programName, NULL);
                exit(1);
            }
            else if (newProcess.pid > 0)
            {
                kill(newProcess.pid, SIGSTOP); // Pausar imediatamente
                rtProcesses[rtCount++] = newProcess;
            }
        }
    }

    scheduleRealTimeProcesses(); // Agendar execução dos processos REAL-TIME
    return 0;
}
