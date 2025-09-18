#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

void *incrementa_vetor(void *args);
void imprime_vetor(long int *vetor, long int tam);
short int inicializa_vetor(long int **vetor, long int tam);
short int verifica_vetor(long int *vetor, long int dim);

typedef struct {
    long int *numeros, inicio, fim;
} argsThread;

void *incrementa_vetor(void *args) {
    argsThread *args_thread = (argsThread *) args;

    for (int i = args_thread->inicio; i < args_thread->fim; i++)
        args_thread->numeros[i]++;

    free(args);
    pthread_exit(NULL);
}

void imprime_vetor(long int *vetor, long int tam) {
    for (long int i = 0; i < tam; i++)
        printf("%ld ", *(vetor + i));
    printf("\n");
}

short int inicializa_vetor(long int **vetor, long int tam) {
    if (!(*vetor = (long int *) calloc(tam, sizeof(long int)))) {
        printf("--Erro ao alocar vetor com calloc().\n");
        return 1;
    }

    for (long int i = 0; i < tam; i++)
        *(*vetor + i) = (i);

    return 0;
}

short int verifica_vetor(long int *vetor, long int dim) {
    for (long int i = 0; i < dim; i++)
        if ((*vetor + i) != i + 1)
            return 1;

    return 0;
}

int main(int argc, char *argv[]) {
    long int num_elementos = 0, num_threads = 0;
    double inicio, fim, delta;

    if (argc < 3) {
        printf("--Argumentos devem ser num_elementos e num_threads.\n");
        return 1;
    }

    num_elementos = atoi(argv[1]);
    num_threads = atoi(argv[2]);
    pthread_t tid[num_threads];

    argsThread *args_thread;
    long int *numeros;
    long int fatia = 0;

    srand(time(NULL));
    if (inicializa_vetor(&numeros, num_elementos)) {
        printf("--Erro ao inicializar o vetor.\n");
        return 2;
    }

    printf("Vetor antes do incremento: ");
    imprime_vetor(numeros, num_elementos);

    for (long int i = 0; i < num_threads; i++) {
        if (!(args_thread = malloc(sizeof(argsThread)))) {
            printf("--Erro na alocacao de args_thread para a thread %ld.\n", i + 1);
            return 3;
        }

        fatia = num_elementos / num_threads;
        args_thread->inicio = i * fatia;
        args_thread->fim = i < num_threads - 1 ? args_thread->inicio + fatia : num_elementos;
        args_thread->numeros = numeros;

        if (pthread_create(&tid[i], NULL, incrementa_vetor, (void *) args_thread)) {
            printf("--Erro na criacao da thread %ld com pthread_create().\n", i + 1);
            return 4;
        }
    }

    GET_TIME(inicio);

    for (long int i = 0; i < num_threads; i++) {
        if (pthread_join(tid[i], NULL)) {
            printf("--Erro no join da thread %ld com pthread_join().\n", i + 1);
            return 5;
        }
    }

    printf("Vetor apos o incremento: ");
    imprime_vetor(numeros, num_elementos);

    GET_TIME(fim);

    if (verifica_vetor(numeros, num_elementos))
        printf("--Erro no processamento do vetor.\n");
    else
        printf("--Sucesso no processamento do vetor.\n");

    delta = fim - inicio;
    printf("Tempo: %lf\n", delta);

    free(numeros);

    return 0;
}