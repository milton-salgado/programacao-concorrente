#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define num_threads 4

void *incrementa_vetor(void *args);
void imprime_vetor(long int *vetor, long int tam);
void inicializa_vetor(long int *vetor, long int tam);

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
        printf("%ld ", vetor[i]);
    printf("\n");
}

void inicializa_vetor(long int *vetor, long int tam) {
    for (long int i = 0; i < tam; i++)
        *(vetor + i) = rand() % 100;
}

int main(int argc, char *argv[]) {
    long int num_elementos = 0;
    long int tam = 0;

    if (argc < 2) {
        printf("--Argumento num_elementos nao foi passado na entrada.\n");
        return 1;
    }

    num_elementos = atoi(argv[1]);
    tam = num_elementos * num_threads;
    pthread_t tid[num_threads];

    argsThread *args_thread;
    long int numeros[tam];
    long int fatia = 0;

    srand(time(NULL));
    inicializa_vetor(numeros, tam);

    printf("Vetor antes do incremento: ");
    imprime_vetor(numeros, tam);

    for (long int i = 0; i < num_threads; i++) {
        if (!(args_thread = malloc(sizeof(argsThread)))) {
            printf("--Erro na alocacao de args_thread para a thread %ld\n", i + 1);
            return 2;
        }

        fatia = tam / num_threads;
        args_thread->inicio = i * fatia;
        args_thread->fim = i < num_threads - 1 ? args_thread->inicio + fatia : tam;
        args_thread->numeros = numeros;

        if (pthread_create(&tid[i], NULL, incrementa_vetor, (void *) args_thread)) {
            printf("--Erro na criacao da thread %ld com pthread_create().\n", i + 1);
            return 3;
        }
    }

    for (long int i = 0; i < num_threads; i++) {
        if (pthread_join(tid[i], NULL)) {
            printf("--Erro no join da thread %ld com pthread_join().\n", i + 1);
            return 4;
        }
    }

    printf("Vetor apos o incremento: ");
    imprime_vetor(numeros, tam);

    return 0;
}