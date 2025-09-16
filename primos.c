#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static int n = 5000000;
static int chunk = 5000;
int next = 0;
int cont = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static bool is_prime(int numero){
    if(numero <= 1) return false;
    if(numero <= 3) return true;
    if(numero % 2 == 0 || numero % 3 == 0) return false;

    for(int i = 5; i*i < numero; i += 2){
        if(numero % i == 0) {
            return false;
        }
    }
    return true;
}

void* intervalo(void* arg){
    int start, end;
    while (next != n){
        
        pthread_mutex_lock(&lock);
        start = next; 
        next += chunk;
        pthread_mutex_unlock(&lock);

        if (start >= n) break; // terminou

        end = (next > n) ? n : next;

        for (int i = start; i < end; i++){
            if (is_prime(i)){
                pthread_mutex_lock(&lock);
                cont++;
                pthread_mutex_unlock(&lock);
            }
        }
    }
}

int main() {
    int k = 4;
    pthread_t threads[k];

    for (int i = 0; i < k; i++){
        pthread_create(&threads[i], NULL, intervalo, NULL);   
    }

    for (int i = 0; i < k; i++){
        pthread_join(threads[i], NULL);
    }

    printf("%d", cont);
    pthread_mutex_destroy(&lock); // destrói o mutex

    return 0;
}