#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define N 5000000
#define CHUNK 5000

int next = 1;
long long cont = 0;

pthread_mutex_t next_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t total_mutex = PTHREAD_MUTEX_INITIALIZER;


static bool is_prime(int numero){
    if(numero <= 1) return false;
    if(numero <= 3) return true;
    if(numero % 2 == 0 || numero % 3 == 0) return false;

    for(int i = 5; i*i <= numero; i += 2){
        if(numero % i == 0) {
            return false;
        }
    }
    return true;
}

void* intervalo(void* arg){
    int start, end;
    while (1){
        pthread_mutex_lock(&next_mutex);
        start = next;
        if (start > N) { pthread_mutex_unlock(&next_mutex); break; }
        end = start + CHUNK - 1;
        if (end > N) end = N;
        next = end + 1;
        pthread_mutex_unlock(&next_mutex);

        long long local = 0;
        for (int i = start; i <= end; i++)
            if (is_prime(i)) local++;

        pthread_mutex_lock(&total_mutex);
        cont += local;
        pthread_mutex_unlock(&total_mutex);
    }
}

void* progresso(void* arg) {
    int total_chunks = (N + CHUNK - 1) / CHUNK;
    const int bar_width = 50; // largura da barra

    while (1) {
        pthread_mutex_lock(&next_mutex);
        int ns = next;
        pthread_mutex_unlock(&next_mutex);

        int proc = (ns - 1) / CHUNK;
        if (proc > total_chunks) proc = total_chunks;

        double pct = (100.0 * proc) / total_chunks;
        int filled = (int)(bar_width * pct / 100.0);

        // imprime a barra
        printf("\r[");
        for (int i = 0; i < bar_width; i++) {
            if (i < filled) printf("=");
            else printf(" ");
        }
        printf("] %6.2f%%", pct);
        fflush(stdout);

        if (proc >= total_chunks) break;
        struct timespec ts = {0, 500000000};
        nanosleep(&ts, NULL);
    }
    printf("\n");
}

double executa(int k, char *label, long long *qntPrimos) {
    next = 0;
    cont = 0;

    pthread_t threads[k];
    pthread_t tprog;

    printf("\n-- Executando com %s!\n\n", label);
    
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    pthread_create(&tprog, NULL, progresso, NULL);
    for (int i = 0; i < k; i++) pthread_create(&threads[i], NULL, intervalo, NULL);
    for (int i = 0; i < k; i++) pthread_join(threads[i], NULL);
    pthread_join(tprog, NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double tempo_ms = (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1e6;

    printf("Tempo de execucao: %.2f ms\n", tempo_ms);
    printf("Quantidade de primos: %lld\n", cont);

    *qntPrimos = cont;

    return tempo_ms;
}

int main(int argc, char** argv) {
    int k;
    long long qntPrimos, tempoExec = 0;
    if (argc < 2) {
        printf("Use: %s < 1 | 4 | 0 (nucleos totais) / benchmark |>\n", argv[0]);
        return 1;
    } else if (strcmp(argv[1], "1") == 0) // Executa com 1 thread
    {
        int valores[] = {1};
        char label[16];
        snprintf(label, sizeof(label), "%d thread%s", valores[0], valores[0] > 1 ? "s" : "");
        tempoExec = executa(valores[0], label, &qntPrimos);
    } else if (strcmp(argv[1], "4") == 0) // Executa com 4 threads
    {
        int valores[] = {4};
        char label[16];
        snprintf(label, sizeof(label), "%d thread%s", valores[0], valores[0] > 1 ? "s" : "");
        tempoExec = executa(valores[0], label, &qntPrimos);
    }else if (strcmp(argv[1], "6") == 0)
    {
        int valores[] = {6};
        char label[16];
        snprintf(label, sizeof(label), "%d thread%s", valores[0], valores[0] > 1 ? "s" : "");
        tempoExec = executa(valores[0], label, &qntPrimos);
    } else if (strcmp(argv[1], "8") == 0)
    {
        int valores[] = {8};
        char label[16];
        snprintf(label, sizeof(label), "%d thread%s", valores[0], valores[0] > 1 ? "s" : "");
        tempoExec = executa(valores[0], label, &qntPrimos);
    } else if(strcmp(argv[1], "benchmark") == 0)
    {
        printf("\n-- Executando modo benchmark!\n\n");
        
        int valores[] = {1, 2, 4, 6, 8};
        int n = sizeof(valores) / sizeof(valores[0]);

        double tempos[5];
        long long resultados[5];

        for (int i = 0; i < n; i++) {
            char label[16];
            snprintf(label, sizeof(label), "%d thread%s", valores[i], valores[i] > 1 ? "s" : "");
            tempos[i] = executa(valores[i], label, &resultados[i]);
            printf("Primos: %lld, Tempo: %.2f ms\n", resultados[i], tempos[i]);
        }

        // depois, imprimir tabela de resumo com speedup
        printf("\nResumo benchmark:\n");
        printf("threads\ttempo_ms\ttotal_primos\tspeedup_vs_k1\n");
        for (int i = 0; i < 5; i++) {
            double speedup = tempos[0] / tempos[i];
            printf("%d     \t%.2f     \t%lld     \t%.2f    \n", valores[i], tempos[i], resultados[i], speedup);
        }

    } else {    // Executa com o numero de cpus disponíveis
        k = (int) sysconf(_SC_NPROCESSORS_ONLN);
        printf("\n Nucleos: %d\n", k);

        int valores[] = {k};
        char label[16];
        snprintf(label, sizeof(label), "%d thread%s", valores[0], valores[0] > 1 ? "s" : "");
        tempoExec = executa(valores[0], label, &qntPrimos);
    }
    
    return 0;
}