#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

void *dobra_id(void *args) {
    long int id_thread = (long int) args, retorno = id_thread * 2;
    pthread_exit((void *) retorno);
}

int main(int argc, char *argv[]) {
    int n_threads; // quantidade de threads que serao criadas (recebidas do terminal)
    long int retorno;
    pthread_t *threads;

    if (argc < 2) {
        printf("--ERRO: Informe a quantidade de threads, no formato: <%s> <n_threads>\n", argv[0]);
        return 1;
    }
    n_threads = atoi(argv[1]);

    if (!((threads = (pthread_t *) calloc(n_threads, sizeof(pthread_t))))) {
        printf("--ERRO: Erro ao alocar vetor de threads com calloc()\n");
        return 2;
    }

    for (long int i = 0; i < n_threads; i++) {
        printf("Criacao da thread %ld\n", i);

        if (pthread_create(&threads[i], NULL, dobra_id, (void *) i)) {
            printf("--ERRO: Erro no pthread_create() da thread %ld\n", i);
            return 3;
        }
    }

    for (long int i = 0; i < n_threads; i++) {
        if (pthread_join(threads[i], (void **) &retorno))
            printf("--ERRO: Erro no pthread_join() da thread %ld\n", i);
        printf("A thread %ld retornou %ld\n", i, retorno);
    }

    printf("Thread principal terminou\n");

    return 0;
}