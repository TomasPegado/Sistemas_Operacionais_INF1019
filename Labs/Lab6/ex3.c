#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXFILA 8
#define PRODUCER_COUNT 64
#define CONSUMER_COUNT 64

int buffer[MAXFILA];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

void *producer(void *arg) {
    for (int i = 1; i <= PRODUCER_COUNT; i++) {
        sleep(1); // Produz a cada 1 segundo
        pthread_mutex_lock(&mutex);
        while (count == MAXFILA) {
            pthread_cond_wait(&not_full, &mutex);
        }
        buffer[in] = i;
        in = (in + 1) % MAXFILA;
        count++;
        printf("Produced: %d\n", i);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *consumer(void *arg) {
    for (int i = 1; i <= CONSUMER_COUNT; i++) {
        sleep(2); // Consome a cada 2 segundos
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }
        int item = buffer[out];
        out = (out + 1) % MAXFILA;
        count--;
        printf("Consumed: %d\n", item);
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t producer_thread, consumer_thread;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return 0;
}
