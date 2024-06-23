#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#define NUM_THREADS 5
#define PULO_MAXIMO 100
#define DESCANSO_MAXIMO 1
#define DISTANCIA_PARA_CORRER 100

static int classificacao = 1;
static pthread_mutex_t lock;
static char *resp[200];
static int cont = 0;

void *Correr(void *sapo);

int main()
{
    pthread_mutex_init(&lock, NULL); // Inicializa o mutex

    classificacao = 1;
    pthread_t threads[NUM_THREADS];
    int t;
    printf("Corrida iniciada ... \n");
    for (t = 0; t < NUM_THREADS; t++)
        pthread_create(&threads[t], NULL, Correr, (void *)(long)t);

    for (t = 0; t < NUM_THREADS; t++)
        pthread_join(threads[t], NULL);

    printf("\nAcabou!!\n");

    pthread_mutex_destroy(&lock); // Destrói o mutex
    pthread_exit(NULL);
}

void *Correr(void *sapo)
{
    int pulos = 0;
    int distanciaJaCorrida = 0;
    while (distanciaJaCorrida <= DISTANCIA_PARA_CORRER)
    {
        int pulo = rand() % PULO_MAXIMO;
        distanciaJaCorrida += pulo;
        pulos++;
        printf("Sapo %ld pulou\n", (long)sapo);
        int descanso = rand() % DESCANSO_MAXIMO;
        sleep(descanso);
    }

    pthread_mutex_lock(&lock); // Inicia seção crítica
    printf("Sapo %ld chegou na posicao %d com %d pulos\n", (long)sapo, classificacao, pulos);
    classificacao++;
    pthread_mutex_unlock(&lock); // Finaliza seção crítica

    pthread_exit(NULL);
}
