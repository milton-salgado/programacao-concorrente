#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

/* ========== DECLARACAO DAS FUNCOES ========== */
int eh_primo(long long int n);
void *produtor(void *arg);
void *consumidor(void *arg);
void libera_recursos_basicos(void);
void libera_recursos_completos(void);
int inicializa_semaforos(void);
void destroi_semaforos(void);

/* ========== VARIAVEIS GLOBAIS ========== */

/* Buffer compartilhado */
int *buffer = NULL;

/* Parametros do problema */
int n_elementos;           /* N: total de elementos a produzir */
int tamanho_buffer;        /* M: tamanho do buffer circular */
int n_consumidores;        /* Numero de threads consumidoras */

/* Controle do buffer */
int contador = 0;          /* Elementos atualmente no buffer */
int total_consumido = 0;   /* Total de elementos ja retirados por TODOS os consumidores */

/* ========== SEMAFOROS ========== */
sem_t slot_vazio;          /* Controla se o buffer esta vazio (produtor pode inserir) */
sem_t slot_cheio;          /* Controla quantidade de elementos disponiveis */
sem_t mutex_prod;          /* Exclusao mutua para o produtor */
sem_t mutex_cons;          /* Exclusao mutua para os consumidores */
sem_t mutex_total;         /* Exclusao mutua para o contador total */

/* ========== ESTRUTURAS DE DADOS ========== */
int *primos_thread = NULL;       /* Array com contagem de primos por thread */
pthread_t *tid_cons = NULL;      /* Array com TIDs das threads consumidoras */

/* ========== FUNCAO: VERIFICA SE UM NUMERO EH PRIMO ========== */
int eh_primo(long long int n) {
    int i;

    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;

    for (i = 3; i < sqrt(n) + 1; i += 2) {
        if (n % i == 0) return 0;
    }

    return 1;
}

/* ========== FUNCAO: LIBERA RECURSOS BASICOS ========== */
void libera_recursos_basicos(void) {
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    if (primos_thread != NULL) {
        free(primos_thread);
        primos_thread = NULL;
    }

    if (tid_cons != NULL) {
        free(tid_cons);
        tid_cons = NULL;
    }
}

/* ========== FUNCAO: DESTROI SEMAFOROS ========== */
void destroi_semaforos(void) {
    sem_destroy(&slot_vazio);
    sem_destroy(&slot_cheio);
    sem_destroy(&mutex_prod);
    sem_destroy(&mutex_cons);
    sem_destroy(&mutex_total);
}

/* ========== FUNCAO: LIBERA TODOS OS RECURSOS ========== */
void libera_recursos_completos(void) {
    destroi_semaforos();
    libera_recursos_basicos();
}

/* ========== FUNCAO: INICIALIZA SEMAFOROS ========== */
int inicializa_semaforos(void) {
    /* Inicializa slot_vazio com 1 (produtor pode inserir) */
    if (sem_init(&slot_vazio, 0, 1) != 0) {
        printf("--ERRO: Falha na inicializacao do semaforo slot_vazio\n");
        return 0;
    }

    /* Inicializa slot_cheio com 0 (buffer vazio) */
    if (sem_init(&slot_cheio, 0, 0) != 0) {
        printf("--ERRO: Falha na inicializacao do semaforo slot_cheio\n");
        sem_destroy(&slot_vazio);
        return 0;
    }

    /* Inicializa mutex_prod com 1 */
    if (sem_init(&mutex_prod, 0, 1) != 0) {
        printf("--ERRO: Falha na inicializacao do semaforo mutex_prod\n");
        sem_destroy(&slot_vazio);
        sem_destroy(&slot_cheio);
        return 0;
    }

    /* Inicializa mutex_cons com 1 */
    if (sem_init(&mutex_cons, 0, 1) != 0) {
        printf("--ERRO: Falha na inicializacao do semaforo mutex_cons\n");
        sem_destroy(&slot_vazio);
        sem_destroy(&slot_cheio);
        sem_destroy(&mutex_prod);
        return 0;
    }

    /* Inicializa mutex_total com 1 */
    if (sem_init(&mutex_total, 0, 1) != 0) {
        printf("--ERRO: Falha na inicializacao do semaforo mutex_total\n");
        sem_destroy(&slot_vazio);
        sem_destroy(&slot_cheio);
        sem_destroy(&mutex_prod);
        sem_destroy(&mutex_cons);
        return 0;
    }

    return 1;
}

/* ========== THREAD PRODUTORA ========== */
/* Insere TODOS os M elementos do buffer de uma vez */
/* Aguarda o buffer esvaziar antes de inserir novamente */
void *produtor(void *arg) {
    static int val_gerado = 0;
    int elementos_inserir;

    (void) arg;

    printf("--LOG: Thread produtora iniciada\n");

    while (val_gerado < n_elementos) {
        /* Aguarda buffer ficar vazio */
        sem_wait(&slot_vazio);
        sem_wait(&mutex_prod);

        /* Calcula quantos elementos inserir neste lote */
        elementos_inserir = tamanho_buffer;
        if (val_gerado + elementos_inserir > n_elementos) {
            elementos_inserir = n_elementos - val_gerado;
        }

        printf("--LOG: Produtor vai inserir %d elementos de uma vez\n", elementos_inserir);

        /* Insere M elementos (ou menos se for o ultimo lote) */
        for (int i = 0; i < elementos_inserir; i++) {
            val_gerado++;
            buffer[i] = val_gerado;
            printf("--LOG: Produtor inseriu %d na posicao %d\n", buffer[i], i);
        }

        contador = elementos_inserir;

        sem_post(&mutex_prod);

        /* Sinaliza elementos disponiveis para consumo */
        for (int i = 0; i < elementos_inserir; i++) {
            sem_post(&slot_cheio);
        }

        printf("--LOG: Produtor inseriu total de %d elementos (ja produziu %d/%d)\n",
            elementos_inserir, val_gerado, n_elementos);
    }

    printf("--LOG: Thread produtora finalizou\n");
    pthread_exit(NULL);
}

/* ========== THREAD CONSUMIDORA ========== */
/* Retira UM elemento por vez do buffer */
/* Cada consumidor compete pelos N elementos totais */
void *consumidor(void *arg) {
    long id = (long) arg;
    int elemento;
    int primos_encontrados = 0;
    int item_consumido_local = 0;
    int item_atual_total;
    static int out = 0;

    /* Validacao do ID */
    if (id < 0 || id >= n_consumidores) {
        printf("--ERRO: ID de consumidor invalido\n");
        pthread_exit(NULL);
    }

    printf("--LOG: Thread consumidora %ld iniciada\n", id);

    /* Loop ate que todos os N elementos sejam consumidos */
    while (1) {
        /* Verifica se todos os elementos ja foram consumidos */
        sem_wait(&mutex_total);
        if (total_consumido >= n_elementos) {
            sem_post(&mutex_total);
            break;
        }
        total_consumido++;
        item_atual_total = total_consumido;
        item_consumido_local++;
        sem_post(&mutex_total);

        /* Aguarda elemento disponivel no buffer */
        sem_wait(&slot_cheio);
        sem_wait(&mutex_cons);

        /* Retira elemento do buffer */
        elemento = buffer[out];
        printf("--LOG: Consumidor %ld retirou %d da posicao %d (item total %d/%d, item local %d)\n",
            id, elemento, out, item_atual_total, n_elementos, item_consumido_local);

        out = (out + 1) % tamanho_buffer;
        contador--;

        /* Se buffer esvaziou, libera o produtor */
        if (contador == 0) {
            printf("--LOG: Buffer esvaziado, liberando produtor\n");
            out = 0;
            sem_post(&slot_vazio);
        }

        sem_post(&mutex_cons);

        /* Verifica primalidade fora da regiao critica */
        if (eh_primo((long long int)elemento)) {
            primos_encontrados++;
            printf("--LOG: Consumidor %ld encontrou primo: %d (total: %d)\n",
                id, elemento, primos_encontrados);
        }
    }

    primos_thread[id] = primos_encontrados;

    printf("--LOG: Thread consumidora %ld finalizou com %d primos\n",
        id, primos_encontrados);
    pthread_exit(NULL);
}

/* ========== FUNCAO PRINCIPAL ========== */
int main(int argc, char *argv[]) {
    pthread_t tid_prod;
    int i;
    int total_primos = 0;
    int max_primos = 0;
    int thread_vencedora = 0;

    /* ===== VALIDACAO DOS ARGUMENTOS ===== */
    if (argc != 4) {
        printf("--ERRO: Numero incorreto de argumentos\n");
        printf("Uso: %s <N> <M> <nConsumidores>\n", argv[0]);
        printf("  N: numero de elementos a produzir\n");
        printf("  M: tamanho do buffer (M << N)\n");
        printf("  nConsumidores: numero de threads consumidoras\n");
        return 1;
    }

    n_elementos = atoi(argv[1]);
    tamanho_buffer = atoi(argv[2]);
    n_consumidores = atoi(argv[3]);

    /* Valida N */
    if (n_elementos <= 0) {
        printf("--ERRO: N deve ser maior que 0\n");
        return 1;
    }

    /* Valida M */
    if (tamanho_buffer <= 0) {
        printf("--ERRO: M deve ser maior que 0\n");
        return 1;
    }

    /* Valida relacao M << N */
    if (tamanho_buffer > n_elementos) {
        printf("--ERRO: M deve ser menor ou igual a N\n");
        return 1;
    }

    /* Valida numero de consumidores */
    if (n_consumidores <= 0) {
        printf("--ERRO: Numero de consumidores deve ser maior que 0\n");
        return 1;
    }

    /* ===== MENSAGENS INICIAIS ===== */
    printf("--LOG: Iniciando com N=%d, M=%d, Consumidores=%d\n",
        n_elementos, tamanho_buffer, n_consumidores);
    printf("--LOG: Produtor insere TODOS os M elementos de uma vez\n");
    printf("--LOG: Consumidores retiram um por vez\n");

    /* ===== ALOCACAO DE MEMORIA ===== */

    /* Aloca buffer */
    buffer = (int *) malloc(tamanho_buffer * sizeof(int));
    if (buffer == NULL) {
        printf("--ERRO: Falha na alocacao do buffer\n");
        return 1;
    }

    /* Aloca array de contagem de primos */
    primos_thread = (int *) calloc(n_consumidores, sizeof(int));
    if (primos_thread == NULL) {
        printf("--ERRO: Falha na alocacao do array de primos\n");
        libera_recursos_basicos();
        return 1;
    }

    /* Aloca array de threads consumidoras */
    tid_cons = (pthread_t *) malloc(n_consumidores * sizeof(pthread_t));
    if (tid_cons == NULL) {
        printf("--ERRO: Falha na alocacao do array de threads\n");
        libera_recursos_basicos();
        return 1;
    }

    /* ===== INICIALIZACAO DOS SEMAFOROS ===== */
    if (!inicializa_semaforos()) {
        libera_recursos_basicos();
        return 1;
    }

    /* ===== CRIACAO DAS THREADS ===== */

    /* Cria thread produtora */
    if (pthread_create(&tid_prod, NULL, produtor, NULL) != 0) {
        printf("--ERRO: Falha na criacao da thread produtora\n");
        libera_recursos_completos();
        return 1;
    }

    /* Cria threads consumidoras */
    for (i = 0; i < n_consumidores; i++) {
        if (pthread_create(&tid_cons[i], NULL, consumidor, (void *) (long) i) != 0) {
            printf("--ERRO: Falha na criacao da thread consumidora %d\n", i);
            pthread_join(tid_prod, NULL);
            for (int j = 0; j < i; j++) {
                pthread_join(tid_cons[j], NULL);
            }
            libera_recursos_completos();
            return 1;
        }
    }

    /* ===== AGUARDA FINALIZACAO DAS THREADS ===== */

    /* Aguarda thread produtora */
    pthread_join(tid_prod, NULL);

    /* Aguarda threads consumidoras */
    for (i = 0; i < n_consumidores; i++) {
        pthread_join(tid_cons[i], NULL);
    }

    /* ===== CALCULO DOS RESULTADOS ===== */
    for (i = 0; i < n_consumidores; i++) {
        total_primos += primos_thread[i];
        if (primos_thread[i] > max_primos) {
            max_primos = primos_thread[i];
            thread_vencedora = i;
        }
    }

    /* ===== EXIBICAO DOS RESULTADOS ===== */
    printf("\n=== RESULTADOS ===\n");
    printf("--OUT: Total de numeros primos encontrados: %d\n", total_primos);
    printf("--OUT: Thread consumidora VENCEDORA: %d (encontrou %d primos)\n",
        thread_vencedora, max_primos);

    /* ===== LIBERACAO DE RECURSOS ===== */
    libera_recursos_completos();

    printf("--LOG: Programa finalizado com sucesso\n");

    return 0;
}