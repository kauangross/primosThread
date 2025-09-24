// gcc -pthread -o corrida corrida.c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <stdint.h>
#include <time.h>

#define NUM_THREADS 8

int Ingressos = 50000000;
int Vendidos = 0;
int Auditoria = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* worker(void* arg) {
    (void)arg;

    int local_sum = 0;

    while (1) {
        pthread_mutex_lock(&lock);

        if (Ingressos <= 0) {
            pthread_mutex_unlock(&lock);
            break; // acabou
        }

        int v = (Ingressos >= 5) ? (1 + rand() % 5) : Ingressos;

        Ingressos -= v;
        Vendidos  += v;
        local_sum += v;

        pthread_mutex_unlock(&lock);
    }

    pthread_mutex_lock(&lock);
    Auditoria += local_sum;
    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void) {
    pthread_t th[NUM_THREADS];

    srand(time(NULL)); // inicializa gerador aleatório só uma vez

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&th[i], NULL, worker, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(th[i], NULL);

    printf("Ingressos = %d - Vendidos: %d - Auditoria: %d\n",
           Ingressos, Vendidos, Auditoria);

    return 0;
}
