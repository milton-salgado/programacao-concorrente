#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int id_thread;
    int *numeros;
    int tam;
} argsThread;

void imprime_numeros(int *numeros, int n);
void *incrementa_numeros(void *args);

void *incrementa_numeros(void *args) {
    argsThread *args_thread = (argsThread *) args;

    printf("Thread %d iniciada.\n", args_thread->id_thread);
    printf("Números antes da thread %d: ", args_thread->id_thread);
    imprime_numeros(args_thread->numeros, args_thread->tam);

    int inicio = (args_thread->id_thread - 1) * (args_thread->tam / 4);
    int fim = (args_thread->tam / 4) * args_thread->id_thread;

    for (int i = inicio; i < fim; i++)
        args_thread->numeros[i]++;

    printf("Números apos a thread %d: ", args_thread->id_thread);
    imprime_numeros(args_thread->numeros, args_thread->tam);
    free(args);

    pthread_exit(NULL);
}

void imprime_numeros(int *numeros, int n) {
    for (int i = 0; i < n; i++)
        printf("%d ", numeros[i]);
    printf("\n");
}

int main(int argc, char *argv[]) {
    const int num_threads = 4;
    int n = 0;
    argsThread *args;

    srand(time(NULL));

    if (argc < 2) {
        printf("Parametro n nao foi recebido.\n");
        return 1;
    }
    n = atoi(argv[1]);

    pthread_t threads[num_threads];
    int tam = num_threads * n;
    int numeros[tam];

    for (int i = 0; i < tam; i++)
        numeros[i] = rand() % 100;

    printf("Numeros iniciais: ");
    imprime_numeros(numeros, tam);

    for (long int i = 0; i < num_threads; i++) {
        printf("Criando a thread %d...\n", i + 1);

        if (!(args = malloc(sizeof(argsThread)))) {
            printf("Erro de alocacao para args.\n");
            return 2;
        }

        args->id_thread = i + 1;
        args->numeros = numeros;
        args->tam = tam;

        if (pthread_create(&threads[i], NULL, incrementa_numeros, (void *) args)) {
            printf("Erro na criacao da thread %d com pthread_create().\n", i + 1);
            return 3;
        }
    }

    for (long int i = 0; i < num_threads; i++)
        if (pthread_join(threads[i], NULL))
            printf("Erro no pthread_join() da thread %d\n");

    printf("Numeros finais: ");
    imprime_numeros(numeros, tam);

    printf("Thread principal terminou.\n");
    // pthread_exit(NULL);
}