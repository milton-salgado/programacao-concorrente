#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

typedef struct {
    int id_thread, n_threads;
} argsThread;

typedef struct {
    int id_thread, resultado;
} retThread;

void *dobra_id(void *args) {
    retThread *retorno; // estrutura de retorno
    argsThread *args_thread = (argsThread *) args; // estrutura dos argumentos

    if (!(retorno = (retThread *) malloc(sizeof(retThread)))) {
        printf("--ERRO: Erro ao alocar estrutura de retorno para a thread %d com malloc()", args_thread->id_thread);
        pthread_exit(NULL); // retorna nulo pois o retorno não foi alocado corretamente
    }

    retorno->id_thread = args_thread->id_thread;
    retorno->resultado = args_thread->id_thread * 2;

    pthread_exit((void *) retorno);
}

int main(int argc, char *argv[]) {
    int n_threads; // quantidade de threads que serao criadas (recebidas do terminal)
    pthread_t *threads; // identificadores das threads no sistema
    argsThread *args_thread; // recebera os argumentos para a thread
    retThread *retorno; // recebera o retorno das threads (nao necessita de alocacao)

    if (argc < 2) {
        printf("--ERRO: Informe a quantidade de threads, no formato: <%s> <n_threads>\n", argv[0]);
        return 1;
    }
    n_threads = atoi(argv[1]);

    if (!((threads = (pthread_t *) calloc(n_threads, sizeof(pthread_t))))) {
        printf("--ERRO: Erro ao alocar vetor de threads com calloc()\n");
        return 2;
    }

    for (int i = 0; i < n_threads; i++) {
        printf("Criacao da thread %d\n", i);

        if (!(args_thread = (argsThread *) malloc(sizeof(args_thread)))) {
            printf("--ERRO: Erro de alocacao na estrutura de argumentos da thread com malloc()\n");
            return 3;
        }

        args_thread->id_thread = i;
        args_thread->n_threads = n_threads;

        if (pthread_create(&threads[i], NULL, dobra_id, (void *) args_thread)) {
            printf("--ERRO: Erro no pthread_create() da thread %d\n", i);
            return 3;
        }
    }

    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(threads[i], (void **) &retorno))
            printf("--ERRO: Erro no pthread_join() da thread %d\n", i);

        printf("A thread %d retornou %d\n", retorno->id_thread, retorno->resultado);
        free(retorno); // libera a memoria alocada para o retorno da thread
    }

    printf("Thread principal terminou\n");

    return 0;
}