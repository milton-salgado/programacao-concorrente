#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    long int id_thread, num_threads;
} argsThread;

typedef struct {
    long int id_thread, aux;
} retThread;

void *print_hello(void *args) {
    retThread *ret_thread;
    argsThread *args_thread = (argsThread *) args;

    if (!(ret_thread = malloc(sizeof(retThread)))) {
        printf("--ERRO: Falha ao alocar a estrutura de retorno da thread %ld\n", args_thread->id_thread);
        pthread_exit(NULL);
    }

    ret_thread->id_thread = args_thread->id_thread;
    ret_thread->aux = args_thread->id_thread * 2;
    free(args);

    pthread_exit((void *) ret_thread);
}


int main(int argc, char *argv[]) {
    argsThread *args_thread;
    retThread *ret_thread;

    long int num_threads;
    pthread_t *tid;

    if (argc < 2) {
        printf("--ERRO: Informe num_threads, escrevendo: <%s> <num_threads>\n", argv[0]);
        return 1;
    }
    num_threads = atoi(argv[1]);

    if (!(tid = (pthread_t *) calloc(num_threads, sizeof(pthread_t)))) {
        printf("--ERRO: Alocacao do vetor de threads falhou com calloc().\n");
        return 2;
    }

    for (long int i = 0; i < num_threads; i++) {
        if (!(args_thread = malloc(sizeof(argsThread)))) {
            printf("--ERRO: Falha ao alocar a estrutura de argumentos da thread %ld\n", i);
            return 3;
        }

        args_thread->id_thread = i;
        args_thread->num_threads = num_threads;

        if (pthread_create(&tid[i], NULL, print_hello, (void *) args_thread)) {
            printf("--ERRO: Falha ao criar a thread %ld com pthread_create().\n", i);
            return 4;
        }
    }

    for (long int i = 0; i < num_threads; i++) {
        if (pthread_join(tid[i], (void **) &ret_thread))
            printf("--ERRO: Falha ao combinar a thread %ld com pthread_join().\n", i);
        printf("Thread %ld retornou %ld.\n", ret_thread->id_thread, ret_thread->aux);
        free(ret_thread);
    }

    printf("--Thread principal terminou.");
    return 0;
}