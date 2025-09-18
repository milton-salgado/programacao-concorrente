#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h> 
#include "timer.h"

#define VERSOES

// variaveis globais
// vetor de elementos
float *vetor;

// estrutura de dados para passar argumentos para a thread
typedef struct {
    long int n;
    int n_threads;
    int id_thread;
} argsThread;

// estrutura de dados para retorno da thread
typedef struct {
    int id_thread;
    float soma_local;
} retThread;

// funcao executada pelas threads
// estrategia de divisao de tarefas: blocos de n/n_threads elementos
void *soma_vetor(void *args) {
    argsThread *args_thread = (argsThread *) args;
    retThread *retorno;
    int ini, fim, fatia;
    float soma_local = 0;

    if (!(retorno = (retThread *) malloc(sizeof(retThread)))) {
        printf("--ERRO: Erro ao alocar estrutura de retorno para a thread %d com malloc()\n", args_thread->id_thread);
        pthread_exit(NULL);
    }

    fatia = args_thread->n / args_thread->n_threads;
    ini = args_thread->id_thread * fatia;
    fim = ini + fatia;
    if (args_thread->id_thread == (args_thread->n_threads - 1))
        fim = args_thread->n;

    for (int i = ini; i < fim; i++) {
        soma_local += vetor[i];
    }

    retorno->id_thread = args_thread->id_thread;
    retorno->soma_local = soma_local;

    pthread_exit((void *) retorno);
}

int main(int argc, char *argv[]) {
    long int n; // tamanho do vetor
    int n_threads; // numero de threads
    FILE *arquivo; // arquivo de entrada
    size_t ret; // retorno da funcao de leitura no arquivo de entrada
    double soma_original; // soma registrada no arquivo
    pthread_t *threads; // vetor de identificadores das threads no sistema
    argsThread *args_thread; // estrutura de argumentos para as threads
    retThread *retorno; // estrutura de retorno das threads

#ifdef VERSOES
    float soma_sequencial, soma_sequencial_blocos; // resultados das somas adicionais
    float soma1, soma2; // auxiliares para a soma sequencial alternada
#endif
    float soma_concorrente = 0; // resultado da soma concorrente

    if (argc < 3) {
        printf("--ERRO: Informe os parametros, no formato: <%s> <arquivo_entrada> <numero_threads>\n", argv[0]);
        return 1;
    }

    if (!(arquivo = fopen(argv[1], "rb"))) {
        printf("--ERRO: Erro na abertura do arquivo para leitura com fopen()\n");
        return 2;
    }

    ret = fread(&n, sizeof(long int), 1, arquivo);
    if (!ret) {
        printf("--ERRO: Erro de leitura das dimensoes do vetor no arquivo\n");
        return 3;
    }

    if (!(vetor = (float *) malloc(sizeof(float) * n))) {
        printf("--ERRO: Erro ao alocar vetor com malloc()\n");
        return 4;
    }

    ret = fread(vetor, sizeof(float), n, arquivo);
    if (ret < n) {
        printf("--ERRO: Erro de leitura dos elementos do vetor no arquivo\n");
        return 5;
    }

    n_threads = atoi(argv[2]);
    if (n_threads > n) n_threads = n;

    if (!(threads = (pthread_t *) malloc(sizeof(pthread_t) * n_threads))) {
        printf("--ERRO: Erro ao alocar vetor de threads com malloc()\n");
        return 6;
    }

    // criacao das threads
    for (int i = 0; i < n_threads; i++) {
        printf("Criacao da thread %d\n", i);

        if (!(args_thread = (argsThread *) malloc(sizeof(argsThread)))) {
            printf("--ERRO: Erro de alocacao na estrutura de argumentos da thread com malloc()\n");
            return 7;
        }

        args_thread->n = n;
        args_thread->n_threads = n_threads;
        args_thread->id_thread = i;

        if (pthread_create(&threads[i], NULL, soma_vetor, (void *) args_thread)) {
            printf("--ERRO: Erro no pthread_create() da thread %d\n", i);
            return 8;
        }
    }

#ifdef VERSOES
    // soma sequencial de tras para frente
    soma_sequencial = 0;
    for (int t = n - 1; t >= 0; t--) {
        soma_sequencial += vetor[t];
    }
    // soma sequencial bloco (== soma com 2 threads)
    soma1 = 0;
    for (int t = 0; t < n / 2; t++) {
        soma1 += vetor[t];
    }
    soma2 = 0;
    for (int t = n / 2; t < n; t++) {
        soma2 += vetor[t];
    }
    soma_sequencial_blocos = soma1 + soma2;
#endif

    // espera todas as threads terminarem e calcula a soma total das threads
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(threads[i], (void **) &retorno)) {
            printf("--ERRO: Erro no pthread_join() da thread %d\n", i);
            return 9;
        }

        printf("A thread %d retornou soma local: %.26f\n", retorno->id_thread, retorno->soma_local);
        soma_concorrente += retorno->soma_local;
        free(retorno); // libera a memoria alocada para o retorno da thread
    }

    // imprime os resultados
    printf("\n");
#ifdef VERSOES
    printf("soma_sequencial (invertida)     = %.26f\n\n", soma_sequencial);
    printf("soma_sequencial_blocos (2 blocos) = %.26f\n\n", soma_sequencial_blocos);
#endif
    printf("soma_concorrente                = %.26f\n", soma_concorrente);

    // le o somatorio registrado no arquivo
    ret = fread(&soma_original, sizeof(double), 1, arquivo);
    printf("\nSoma original                   = %.26lf\n", soma_original);

    printf("Thread principal terminou\n");

    // desaloca os espacos de memoria
    free(vetor);
    free(threads);
    // fecha o arquivo
    fclose(arquivo);

    return 0;
}