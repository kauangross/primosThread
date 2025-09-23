/* primos.c - Conta primos de 1 a N usando k threads
 * gcc -pthread -O2 -o primos primos.c -lm no terminal do Ubuntu
 * ./primos k
 * ./primos benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define N 5000000
#define CHUNK 5000

int next;
pthread_mutex_t next_mutex = PTHREAD_MUTEX_INITIALIZER;

long long total_primos;
pthread_mutex_t total_mutex = PTHREAD_MUTEX_INITIALIZER;

int primo(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if ((n & 1) == 0) return 0;
    for (int d = 3; d * d <= n; d += 2)
        if (n % d == 0) return 0;
    return 1;
}

void* worker(void* arg) {
    (void)arg;
    while (1) {

        int start, end;
        pthread_mutex_lock(&next_mutex);
        start = next;
        if (start > N) {
             pthread_mutex_unlock(&next_mutex); break; }
        end = start + CHUNK - 1;
        if (end > N) 
            end = N;
        next = end + 1;
        pthread_mutex_unlock(&next_mutex);

        long long local = 0;
        for (int v = start; v <= end; v++)
            if (primo(v)) local++;

        pthread_mutex_lock(&total_mutex);
        total_primos += local;
        pthread_mutex_unlock(&total_mutex);
    }
    return NULL;
}

void* progresso(void* arg) {
    int total_chunks = (N + CHUNK - 1) / CHUNK;
    while (1) {
        pthread_mutex_lock(&next_mutex);
        int ns = next;
        pthread_mutex_unlock(&next_mutex);

        int proc = (ns - 1) / CHUNK;
        if (proc > total_chunks) proc = total_chunks;
        double pct = (100.0 * proc) / total_chunks;

        

        printf("\r %6.2f%%", pct);
        fflush(stdout);

        if (proc >= total_chunks) break;
        struct timespec ts = {0, 200000000};
        nanosleep(&ts, NULL);
    }
    printf("\n");
    return NULL;
}

double executar(int k, long long *res_primos) {
    next = 1;
    total_primos = 0;

    pthread_t *threads = malloc(sizeof(pthread_t) * k);
    pthread_t tprog;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    pthread_create(&tprog, NULL, progresso, NULL);
    for (int i = 0; i < k; i++) pthread_create(&threads[i], NULL, worker, NULL);
    for (int i = 0; i < k; i++) pthread_join(threads[i], NULL);
    pthread_join(tprog, NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double ms = (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    *res_primos = total_primos;
    free(threads);
    return ms;
}

int main(int argc, char** argv) {
    if (argc != 2) { fprintf(stderr, "Uso: %s k | benchmark\n", argv[0]); return 1; }

    if (strcmp(argv[1], "benchmark") == 0) {
        int ks[] = {1, 2, 4, 6, 8};
        int nks = sizeof(ks)/sizeof(ks[0]);
        double tempos[nks];
        long long resultados[nks];

        // primeiro, executar todas as threads e armazenar tempos e resultados
        for (int i = 0; i < nks; i++) {
            printf("\nExecutando com %d threads\n", ks[i]);
            tempos[i] = executar(ks[i], &resultados[i]);
            printf("Primos: %lld, Tempo: %.2f ms\n", resultados[i], tempos[i]);
        }

        // depois, imprimir tabela de resumo com speedup
        printf("\nResumo benchmark:\n");
        printf("threads\ttempo_ms\ttotal_primos\tspeedup_vs_k1\n");
        for (int i = 0; i < nks; i++) {
            double speedup = tempos[0] / tempos[i];
            printf("%d     \t%.2f     \t%lld     \t%.2f    \n", ks[i], tempos[i], resultados[i], speedup);
        }

    } else {
        int k = atoi(argv[1]);
        if (k <= 0) k = (int) sysconf(_SC_NPROCESSORS_ONLN);

        long long resultado;
        double tempo = executar(k, &resultado);
        printf("Total de primos: %lld\n", resultado);
        printf("Tempo: %.2f ms\n", tempo);
    }
    return 0;
}