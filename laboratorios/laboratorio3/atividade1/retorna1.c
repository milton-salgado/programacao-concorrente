#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_hello(void *arg) {
    long int id_thread = (long int) arg, retorno = id_thread * 2;
    pthread_exit((void *) retorno);
}

int main(int argc, char *argv[]) {
    long int num_threads;
    long int retorno;
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

    for (long int i = 0; i < num_threads; i++)
        if (pthread_create(&tid[i], NULL, print_hello, (void *) i)) {
            printf("--ERRO: Falha ao criar a thread %ld com pthread_create().\n", i);
            return 3;
        }

    for (long int i = 0; i < num_threads; i++) {
        if (pthread_join(tid[i], (void **) &retorno))
            printf("--ERRO: Falha ao combinar a thread %ld com pthread_join().\n", i);
        printf("Thread %ld retornou %ld.\n", i, retorno);
    }

    printf("--Thread principal terminou.");
    return 0;
}