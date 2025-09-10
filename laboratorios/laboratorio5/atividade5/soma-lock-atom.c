#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

int nthreads; // numero de threads
long long soma = 0; // variavel compartilhada entre as threads (contador)
pthread_mutex_t mutex; // variavel de lock para exclusao mutua
pthread_cond_t cond_multiplo; // variavel condicional: ha multiplo de 1000 pendente para imprimir
pthread_cond_t cond_impresso; // variavel condicional: logger terminou de imprimir

// --estado de coordenacao entre produtoras e logger
long long proximo_multiplo = 0; // valor (soma) que deve ser impresso pela logger
long long ultimo_impresso = 0; // ultimo multiplo efetivamente impresso (evita repeticao)
int multiplo_encontrado = 0; // flag: ha multiplo pendente para a logger
int multiplo_impresso = 0; // flag: logger concluiu a impressao

// --controle de termino
int trabalhadores_ativos = 0; // quantas produtoras (workers) ainda estao rodando
//! precisei adicionar essa variavel para evitar que a thread logger ficasse
//! esperando por novos trabalhos apos todas as produtoras ja terem terminado
//! isso causava um deadlock, e o programa nao terminava na ultima versao que enviei

void *executa_tarefa(void *arg); // threads produtoras: incrementam 'soma' e sinalizam multiplos
void *extra(void *args); // thread logger: imprime os multiplos de 1000 (sem repeticao)

// funcao executada pelas threads produtoras
void *executa_tarefa(void *arg) {
    long long id = (long long) (intptr_t) arg;
    printf("Thread: %lld esta executando...\n", id);

    for (int i = 0; i < 100000; i++) {
        // --entrada na SC
        pthread_mutex_lock(&mutex);

        // --SC (secao critica)
        soma++; // incrementa a variavel compartilhada

        // --se atingiu multiplo de 1000, registra para a logger e aguarda confirmacao de impressao
        if (soma % 1000 == 0) {
            proximo_multiplo = soma; // captura o valor ainda protegido pelo mesmo mutex
            multiplo_encontrado = 1; // ha trabalho para a logger
            multiplo_impresso = 0; // ainda nao foi impresso

            // --sinaliza para a thread extra que ha um multiplo de 1000
            pthread_cond_signal(&cond_multiplo);

            // --aguarda a logger imprimir esse exato multiplo
            while (!multiplo_impresso || ultimo_impresso != proximo_multiplo)
                pthread_cond_wait(&cond_impresso, &mutex);
        }

        // --saida da SC
        pthread_mutex_unlock(&mutex);
    }

    // --sinaliza que esta produtora terminou
    pthread_mutex_lock(&mutex);
    trabalhadores_ativos--;
    // --acorda a logger (ela pode estar esperando por mais trabalho ou pelo fim)
    pthread_cond_signal(&cond_multiplo);
    pthread_mutex_unlock(&mutex);

    printf("Thread : %lld terminou!\n", id);
    return NULL;
}

// funcao da thread logger (impressora dos multiplos)
void *extra(void *args) {
    (void) args;
    printf("Extra : esta executando...\n");

    pthread_mutex_lock(&mutex);
    while (1) {
        // --espera ate haver multiplo pendente OU ate todas as produtoras terminarem
        while (!multiplo_encontrado && trabalhadores_ativos > 0)
            pthread_cond_wait(&cond_multiplo, &mutex);

        // --condicao de saida: ninguem mais produz e nao ha trabalho pendente
        if (!multiplo_encontrado && trabalhadores_ativos == 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // --ha um multiplo pendente, logo imprime apenas se ainda nao foi impresso
        if (multiplo_encontrado && proximo_multiplo != ultimo_impresso) {
            printf("soma = %lld (multiplo de 1000)\n", proximo_multiplo);
            ultimo_impresso = proximo_multiplo;
        }

        // --limpa estado e libera produtoras
        multiplo_encontrado = 0;
        multiplo_impresso = 1;
        pthread_cond_broadcast(&cond_impresso);
        // --volta ao loop para checar se ha mais trabalho ou se terminou
    }

    printf("Extra: terminou!\n");
    return NULL;
}

// fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; // identificadores das threads no sistema

    // --le e avalia os parametros de entrada
    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }

    nthreads = atoi(argv[1]);
    if (nthreads <= 0) {
        printf("--ERRO: numero de threads invalido\n");
        return 1;
    }

    trabalhadores_ativos = nthreads; // inicializa o contador de produtoras vivas

    // --aloca as estruturas
    tid = (pthread_t *) malloc((nthreads + 1) * sizeof(pthread_t));
    if (!tid) {
        printf("--ERRO: malloc()\n");
        return 1;
    }

    // --inicializa o mutex e as variaveis condicionais
    if (pthread_mutex_init(&mutex, NULL)) {
        printf("--ERRO: pthread_mutex_init()\n");
        return 1;
    }

    if (pthread_cond_init(&cond_multiplo, NULL)) {
        printf("--ERRO: pthread_cond_init(cond_multiplo)\n");
        return 1;
    }

    if (pthread_cond_init(&cond_impresso, NULL)) {
        printf("--ERRO: pthread_cond_init(cond_impresso)\n");
        return 1;
    }

    // --cria thread de log primeiro
    // ! precisei trocar a ordem da criacao das threads para evitar que a thread extra
    // ! fosse criada depois que todas as threads trabalhadoras ja tivessem terminado
    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create(logger)\n");
        return 1;
    }

    // --cria as threads trabalhadoras
    for (long long t = 0; t < nthreads; t++)
        if (pthread_create(&tid[t], NULL, executa_tarefa, (void *) (intptr_t) t)) {
            printf("--ERRO: pthread_create(worker)\n");
            return 1;
        }

    // --espera todas as produtoras terminarem
    for (int t = 0; t < nthreads; t++)
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join(worker)\n");
            return 1;
        }

    // --acorda a logger (caso esteja aguardando por novos trabalhos)
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond_multiplo);
    pthread_mutex_unlock(&mutex);

    // --espera a logger terminar
    if (pthread_join(tid[nthreads], NULL)) {
        printf("--ERRO: pthread_join(logger)\n");
        return 1;
    }

    // --finaliza mutex e variaveis condicionais
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_multiplo);
    pthread_cond_destroy(&cond_impresso);

    printf("Valor final de 'soma' = %lld\n", soma);

    free(tid);
    return 0;
}