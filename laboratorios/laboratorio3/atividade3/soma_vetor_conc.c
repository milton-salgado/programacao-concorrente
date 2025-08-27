#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h> 
#include "timer.h"

// variaveis globais
float *vetor; // vetor de elementos

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
    float soma_concorrente = 0; // resultado da soma concorrente

    // variaveis para medicao de tempo
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

    // inicio da medicao do tempo de processamento
    GET_TIME(tempo_inicio);

    // criacao das threads
    for (int i = 0; i < n_threads; i++) {
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

    // espera todas as threads terminarem e calcula a soma total das threads
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(threads[i], (void **) &retorno)) {
            printf("--ERRO: Erro no pthread_join() da thread %d\n", i);
            return 9;
        }

        soma_concorrente += retorno->soma_local;
        free(retorno); // libera a memoria alocada para o retorno da thread
    }

    // fim da medicao do tempo de processamento
    GET_TIME(tempo_fim);
    tempo_processamento = tempo_fim - tempo_inicio;

    // le o somatorio registrado no arquivo
    ret = fread(&soma_original, sizeof(double), 1, arquivo);

    // verifica a corretude do resultado
    double diferenca = soma_concorrente - soma_original;
    int resultado_correto = (diferenca < 1e-5 && diferenca > -1e-5);

    // impressao dos resultados 
    printf("%ld\t%d\t%.6f\t%.6f\t%.6lf\t%s\n",
        n,                           // Dimensão do vetor
        n_threads,                   // Número de threads
        tempo_processamento,         // Tempo de processamento
        soma_concorrente,            // Soma calculada pelas threads
        soma_original,               // Soma original do arquivo
        resultado_correto ? "OK" : "ERRO"); // Corretude

    // desaloca os espacos de memoria
    free(vetor);
    free(threads);
    fclose(arquivo);

    return 0;
}