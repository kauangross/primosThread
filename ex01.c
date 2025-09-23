// gcc -pthread -o corrida corrida.c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <stdint.h>

#define NUM_THREADS 8

int Ingressos = 500;
int Vendidos;
int Auditoria;

static long long contador = 0; // compartilhado entre as threads

void* worker(void* arg) {
    (void)arg;

    int local_sum = 0;
    int v = 0;

    while(Ingressos > 0){
        if (Ingressos < 5){
            v = Ingressos;
            Vendidos += Ingressos;
            Ingressos -= Ingressos;
        } else if (Ingressos >= 5){
            srand(time(NULL));  // inicializa o gerador
            int v = 1 + rand() % 5;
            Vendidos += v;
            Ingressos -= v;
            local_sum += v;
        }
    }
    Auditoria += local_sum;

    return NULL;
}

int main(void) {
    pthread_t th[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&th[i], NULL, worker, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(th[i], NULL);

    printf("Ingressos = %d - Vendidos: %d - Auditoria: %d\n\n", Ingressos, Vendidos, Auditoria);

    return 0;
}