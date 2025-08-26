#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *imprime_id(void *arg) {
    long int id_thread = (long int) arg;
    printf("ID da thread: %d\n", id_thread);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int num_threads;

    if (argc < 2) {
        printf("Parametro num_threads nao foi recebido.\n");
        return 1;
    }
    num_threads = atoi(argv[1]);

    pthread_t threads[num_threads];

    for (long int i = 0; i < num_threads; i++) {
        printf("Criando a thread %d...\n", i + 1);
        if (pthread_create(&threads[i], NULL, imprime_id, (void *) i + 1)) {
            printf("Erro na criacao da thread %d com pthread_create().\n", i + 1);
            return 2;
        }
    }

    printf("Thread principal terminou.\n");
    pthread_exit(NULL);
}