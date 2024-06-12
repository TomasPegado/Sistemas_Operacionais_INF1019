#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXFILA 8
#define PRODUCER_COUNT 64
#define CONSUMER_COUNT 64
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2

int buffer[MAXFILA];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

void *producer(void *arg) {
    for (int i = 1; i <= PRODUCER_COUNT / NUM_PRODUCERS; i++) {
        sleep(1); // Produz a cada 1 segundo
        pthread_mutex_lock(&mutex);
        while (count == MAXFILA) {
            pthread_cond_wait(&not_full, &mutex);
        }
        buffer[in] = i;
        in = (in + 1) % MAXFILA;
        count++;
        printf("Produzido: %d por produtor %ld\n", i, (long)arg);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *consumer(void *arg) {
    for (int i = 1; i <= CONSUMER_COUNT / NUM_CONSUMERS; i++) {
        sleep(2); // Consome a cada 2 segundos
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }
        int item = buffer[out];
        out = (out + 1) % MAXFILA;
        count--;
        printf("Consumido: %d por consumidor %ld\n", item, (long)arg);
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    for (long i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, (void *)i);
    }

    for (long i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, (void *)i);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return 0;
}
