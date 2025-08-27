#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

// variaveis globais
float *vetor1, *vetor2; // vetores de entrada

// estrutura de dados para passar argumentos para a thread
typedef struct {
    long int n;
    int n_threads;
    int id_thread;
} argsThread;

// estrutura de dados para retorno da thread
typedef struct {
    int id_thread;
    double produto_local;
} retThread;

// funcao executada pelas threads - calcula produto interno parcial
void *produto_interno_thread(void *args);

void *produto_interno_thread(void *args) {
    argsThread *args_thread = (argsThread *) args;
    retThread *retorno;
    double produto_local = 0;
    int fatia = args_thread->n / args_thread->n_threads;
    int ini = args_thread->id_thread * fatia;
    int fim = (args_thread->id_thread == args_thread->n_threads - 1) ?
        args_thread->n : ini + fatia;

    if (!(retorno = (retThread *) malloc(sizeof(retThread)))) {
        printf("--ERRO: Erro ao alocar estrutura de retorno para a thread %d com malloc()\n", args_thread->id_thread);
        pthread_exit(NULL);
    }

    // calcula o produto interno parcial
    for (int i = ini; i < fim; i++)
        produto_local += vetor1[i] * vetor2[i];

    retorno->id_thread = args_thread->id_thread;
    retorno->produto_local = produto_local;

    pthread_exit((void *) retorno);
}

int main(int argc, char *argv[]) {
    long int n; // dimensao dos vetores
    int n_threads; // numero de threads
    FILE *arquivo; // arquivo de entrada
    size_t ret; // retorno da funcao de leitura
    double produto_original; // produto interno registrado no arquivo
    pthread_t *threads; // vetor de identificadores das threads
    argsThread *args_thread; // estrutura de argumentos para as threads
    retThread *retorno; // estrutura de retorno das threads
    double produto_concorrente = 0; // resultado da soma concorrente
    double tempo_inicio, tempo_fim, tempo_processamento;

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
        printf("--ERRO: Erro de leitura das dimensoes dos vetores no arquivo\n");
        return 3;
    }

    if (!(vetor1 = (float *) malloc(sizeof(float) * n))) {
        printf("--ERRO: Erro ao alocar primeiro vetor com malloc()\n");
        return 4;
    }

    if (!(vetor2 = (float *) malloc(sizeof(float) * n))) {
        printf("--ERRO: Erro ao alocar segundo vetor com malloc()\n");
        return 5;
    }

    ret = fread(vetor1, sizeof(float), n, arquivo);
    if (ret < n) {
        printf("--ERRO: Erro de leitura do primeiro vetor no arquivo\n");
        return 6;
    }

    ret = fread(vetor2, sizeof(float), n, arquivo);
    if (ret < n) {
        printf("--ERRO: Erro de leitura do segundo vetor no arquivo\n");
        return 7;
    }

    n_threads = atoi(argv[2]);
    if (n_threads > n) n_threads = n;

    if (!(threads = (pthread_t *) malloc(sizeof(pthread_t) * n_threads))) {
        printf("--ERRO: Erro ao alocar vetor de threads com malloc()\n");
        return 8;
    }

    // inicio da medicao do tempo de processamento
    GET_TIME(tempo_inicio);

    // criacao das threads
    for (int i = 0; i < n_threads; i++) {
        if (!(args_thread = (argsThread *) malloc(sizeof(argsThread)))) {
            printf("--ERRO: Erro de alocacao na estrutura de argumentos da thread com malloc()\n");
            return 9;
        }

        args_thread->n = n;
        args_thread->n_threads = n_threads;
        args_thread->id_thread = i;

        if (pthread_create(&threads[i], NULL, produto_interno_thread, (void *) args_thread)) {
            printf("--ERRO: Erro no pthread_create() da thread %d\n", i);
            return 10;
        }
    }

    // espera todas as threads terminarem e calcula o produto interno total
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(threads[i], (void **) &retorno)) {
            printf("--ERRO: Erro no pthread_join() da thread %d\n", i);
            return 11;
        }

        produto_concorrente += retorno->produto_local;
        free(retorno); // libera a memoria alocada para o retorno da thread
    }

    // fim da medicao do tempo de processamento
    GET_TIME(tempo_fim);
    tempo_processamento = tempo_fim - tempo_inicio;

    // le o produto interno registrado no arquivo
    ret = fread(&produto_original, sizeof(double), 1, arquivo);

    // calcula a variacao relativa
    double variacao_relativa = (produto_original != 0) ?
        ((produto_concorrente - produto_original) / produto_original) : 0;

    // impressao dos resultados
    printf("%ld\t%d\t%.6f\t%.6lf\t%.6lf\t%.6lf\n",
        n,                      // Dimensao dos vetores
        n_threads,              // Numero de threads
        tempo_processamento,    // Tempo de processamento
        produto_concorrente,    // Produto interno calculado
        produto_original,       // Produto interno original
        variacao_relativa);     // Variacao relativa

    // desaloca os espacos de memoria
    free(vetor1);
    free(vetor2);
    free(threads);
    fclose(arquivo);

    return 0;
}