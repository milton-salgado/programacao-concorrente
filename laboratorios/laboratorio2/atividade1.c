#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

typedef struct {
    short int id_thread;
    short int num_threads;
    long int dim;
} argsThread;

int *vetor; // vetor que sera processado

void *f(void *args) {
    argsThread *args_thread = (argsThread *) args;
    long int fatia, inicio, fim;

    fatia = args_thread->dim / args_thread->num_threads;
    inicio = args_thread->id_thread * fatia;
    fim = args_thread->id_thread == args_thread->num_threads - 1 ? args_thread->dim : inicio + fatia;

    for (long int i = inicio; i < fim; i++)
        *(vetor + i) += 1;

    free(args); // libera a memoria da estrutura alocada
    pthread_exit(NULL);
}

short int inicializa_vetor(int **vetor, long int dim) {
    if (!(*vetor = (calloc(dim, sizeof(int))))) {
        printf("ERRO de alocacao de memoria\n");
        return 1;
    }

    for (long int i = 0; i < dim; i++)
        *(*vetor + i) = (int) i;

    return 0;
}

short int verifica_vetor(int *vetor, long int dim) {
    for (long int i = 0; i < dim; i++)
        if ((*vetor + i) != i + 1)
            return 1;

    return 0;
}

void imprime_vetor(int *vetor, long int dim) {
    for (long int i = 0; i < dim; i++)
        printf("%d ", *(vetor + i));
    printf("\n");
}

int main(int argc, char *argv[]) {
    short int num_threads; // quantidade de threads de execucao
    long int dim; // dimensao do vetor de entrada
    double inicio, fim, delta; // variaveis de controle de tempo

    if (argc < 3) {
        fprintf(stderr, "ERRO de entrada, digite: <%s> <dimensao do vetor> <quantidade de threads>\n", argv[0]);
        return 1;
    }

    dim = atoi(argv[1]);
    num_threads = atoi(argv[2]);
    pthread_t threads[num_threads];

    if (inicializa_vetor(&vetor, dim)) {
        fprintf(stderr, "ERRO de alocacao de memoria no vetor\n");
        return 2;
    }

    imprime_vetor(vetor, dim);

    argsThread *args;

    GET_TIME(inicio);

    for (short int i = 0; i < num_threads; i++) {
        if (!(args = (argsThread *) malloc(sizeof(argsThread)))) {
            fprintf(stderr, "ERRO de alocacao de memoria na struct\n");
            return 3;
        }

        args->id_thread = i;
        args->num_threads = num_threads;
        args->dim = dim;

        if (pthread_create(&threads[i], NULL, f, (void *) args)) {
            fprintf(stderr, "ERRO de criacao da thread %hd\n", i);
            return 4;
        }
    }

    for (short int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL)) {
            fprintf(stderr, "ERRO de retorno da thread %hd\n", i);
            return 5;
        }
    }

    imprime_vetor(vetor, dim);

    GET_TIME(fim);

    if (verifica_vetor(vetor, dim))
        fprintf(stderr, "ERRO de processamento no vetor\n");
    else
        fprintf(stderr, "Processamento do vetor concluido com sucesso\n");

    delta = fim - inicio;
    printf("Tempo: %lf\n", delta);

    return 0;
}