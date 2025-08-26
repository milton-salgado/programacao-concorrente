#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int id_thread, num_threads;
} argsThread;

void *imprime_info_thread(void *args) {
    argsThread args_thread = *(argsThread *) args;
    printf("ID da thread: %d - Numero de threads: %d\n", args_thread.id_thread, args_thread.num_threads);
    free(args);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int num_threads;
    argsThread *args;

    if (argc < 2) {
        printf("Parametro num_threads nao foi recebido.\n");
        return 1;
    }
    num_threads = atoi(argv[1]);

    pthread_t threads[num_threads];

    for (long int i = 0; i < num_threads; i++) {
        printf("Criando a thread %d...\n", i + 1);

        if (!(args = malloc(sizeof(argsThread)))) {
            printf("Erro de alocacao para args.\n");
            return 2;
        }

        args->id_thread = i + 1;
        args->num_threads = num_threads;

        if (pthread_create(&threads[i], NULL, imprime_info_thread, (void *) args)) {
            printf("Erro na criacao da thread %d com pthread_create().\n", i + 1);
            return 3;
        }
    }

    for (long int i = 0; i < num_threads; i++)
        if (pthread_join(threads[i], NULL))
            printf("Erro no pthread_join() da thread %d\n");

    printf("Thread principal terminou.\n");
    // pthread_exit(NULL);
}