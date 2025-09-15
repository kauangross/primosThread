#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* hello(void* arg) {
    printf("Hello from thread!\n");
    return NULL;
}

int main() {
    pthread_t t;
    if (pthread_create(&t, NULL, hello, NULL)) {
        fprintf(stderr, "Erro criando thread\n");
        return 1;
    }
    pthread_join(t, NULL);
    printf("Thread terminou!\n");
    return 0;
}
