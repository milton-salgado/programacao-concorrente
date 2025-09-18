#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h> 

void *produto_interno_concorrente(void *args);
long int produto_interno_sequencial(long int *vetor1, long int *vetor2, long int tam);
void imprime_vetor(long int *vetor, long int tam);

typedef struct {
    long int id_thread, inicio, fim, *vetor1, *vetor2;
} argsThread;

typedef struct {
    long int produto_parcial;
} retThread;

void *produto_interno_concorrente(void *args) {
    argsThread *args_thread = (argsThread *) args;
    retThread *ret_thread = NULL;
    long int produto_parcial = 0;

    if (!(ret_thread = (retThread *) malloc(sizeof(retThread)))) {
        printf("--ERRO: Alocacao da estrutura de retorno da thread %ld falhou.\n", args_thread->id_thread);
        pthread_exit(NULL);
    }

    for (long int i = args_thread->inicio; i < args_thread->fim; i++)
        produto_parcial += args_thread->vetor1[i] * args_thread->vetor2[i];

    ret_thread->produto_parcial = produto_parcial;
    free(args);
    pthread_exit((void *) ret_thread);
}

long int produto_interno_sequencial(long int *vetor1, long int *vetor2, long int tam) {
    long int soma = 0;
    for (int i = 0; i < tam; i++)
        soma += vetor1[i] * vetor2[i];
    return soma;
}

short int inicializa_vetor(long int **vetor, long int tam) {
    if (!(*vetor = calloc(tam, sizeof(long int)))) {
        printf("--ERRO: Alocacao do vetor recebido em inicializa_vetor() falhou.\n");
        return 1;
    }

    for (long int i = 0; i < tam; i++)
        *(*vetor + i) = i;

    return 0;
}

void imprime_vetor(long int *vetor, long int tam) {
    printf("--LOG: Vetor: ");
    for (long int i = 0; i < tam; i++)
        printf("%ld ", *(vetor + i));
    printf("\n");
}

int main(int argc, char *argv[]) {
    long int num_elementos = 0, num_threads = 0, fatia = 0, produto_total = 0, *vetor1, *vetor2;
    argsThread *args_thread;
    retThread *ret_thread;
    pthread_t *tid;

    if (argc < 3) {
        printf("--ERRO: Falta parametros.\n");
        return 1;
    }

    num_elementos = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    if (inicializa_vetor(&vetor1, num_elementos) || inicializa_vetor(&vetor2, num_elementos)) {
        printf("--ERRO: Alocacao do(s) vetor(es) falhou.\n");
        return 2;
    }

    imprime_vetor(vetor1, num_elementos);
    imprime_vetor(vetor2, num_elementos);

    if (!(tid = (pthread_t *) calloc(num_threads, sizeof(pthread_t)))) {
        printf("--ERRO: Alocacao do vetor de threads falhou.\n");
        return 3;
    }

    for (long int i = 0; i < num_threads; i++) {
        if (!(args_thread = (argsThread *) malloc(sizeof(argsThread)))) {
            printf("--ERRO: Alocacao da estrutura de argumentos da thread %ld falhou.\n", i);
            return 4;
        }

        args_thread->id_thread = i;
        fatia = num_elementos / num_threads;
        args_thread->inicio = i * fatia;
        args_thread->fim = i < num_threads - 1 ? args_thread->inicio + fatia : num_elementos;
        args_thread->vetor1 = vetor1;
        args_thread->vetor2 = vetor2;

        if (pthread_create(&tid[i], NULL, produto_interno_concorrente, (void *) args_thread)) {
            printf("--ERRO: Criacao com pthread_create() da thread %ld falhou.\n", i);
            return 5;
        }
    }

    for (long int i = 0; i < num_threads; i++) {
        if (pthread_join(tid[i], (void **) &ret_thread)) {
            printf("--ERRO: Join com pthread_join() da thread %ld falhou.\n", i);
            return 6;
        }
        produto_total += ret_thread->produto_parcial;
    }

    printf("--LOG: Produto total concorrente: %ld\n", produto_total);
    printf("--LOG: Produto total sequencial: %ld\n", produto_interno_sequencial(vetor1, vetor2, num_elementos));

    return 0;
}