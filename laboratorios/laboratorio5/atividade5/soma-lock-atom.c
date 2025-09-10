#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int nthreads; //numero de threads
long int soma = 0; //variavel compartilhada entre as threads
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_cond_t cond_multiplo; //variavel condicional para sinalizar multiplo de 1000
pthread_cond_t cond_impresso; //variavel condicional para sinalizar que foi impresso
int multiplo_encontrado = 0; //flag para indicar se um multiplo de 1000 foi encontrado
int multiplo_impresso = 0; //flag para indicar se o multiplo foi impresso

void *executa_tarefa(void *arg);
void *extra(void *args);

//funcao executada pelas threads
void *executa_tarefa(void *arg) {
    long int id = (long int) arg;
    printf("Thread : %ld esta executando...\n", id);

    for (int i = 0; i < 100000; i++) {
        //--entrada na SC
        pthread_mutex_lock(&mutex);

        //--SC (seção critica)
        soma++; //incrementa a variavel compartilhada

        //--verifica se soma é múltiplo de 1000
        if (soma % 1000 == 0) {
            multiplo_encontrado = 1;
            multiplo_impresso = 0;

            //--sinaliza para a thread extra que há um múltiplo de 1000
            pthread_cond_signal(&cond_multiplo);

            //--espera até que o múltiplo seja impresso
            while (!multiplo_impresso)
                pthread_cond_wait(&cond_impresso, &mutex);

            multiplo_encontrado = 0;
        }

        //--saida da SC
        pthread_mutex_unlock(&mutex);
    }

    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra(void *args) {
    printf("Extra : esta executando...\n");

    while (1) {
        pthread_mutex_lock(&mutex);

        //--espera até que um múltiplo de 1000 seja encontrado
        while (!multiplo_encontrado)
            pthread_cond_wait(&cond_multiplo, &mutex);

        //--se chegou aqui, há um múltiplo de 1000 para imprimir
        if (soma % 1000 == 0)
            printf("soma = %ld (multiplo de 1000)\n", soma);

        //--sinaliza que o múltiplo foi impresso para liberar as threads que incrementam
        multiplo_impresso = 1;
        pthread_cond_broadcast(&cond_impresso);

        //--aqui eu verifico se deve terminar (quando soma atingir o valor final)
        //--ja que temos nthreads * 100000 incrementos, podemos verificar calculando
        if (soma >= 100000 * nthreads) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);
    }

    printf("Extra : terminou!\n");
    pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; //identificadores das threads no sistema

    //--le e avalia os parametros de entrada
    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    //--aloca as estruturas
    tid = (pthread_t *) malloc(sizeof(pthread_t) * (nthreads + 1));
    if (tid == NULL) { puts("ERRO--malloc"); return 2; }

    //--inicializa o mutex e as variáveis condicionais
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_multiplo, NULL);
    pthread_cond_init(&cond_impresso, NULL);

    //--cria thread de log primeiro
    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n"); exit(-1);
    }

    //--cria as threads trabalhadoras
    for (long int t = 0; t < nthreads; t++) {
        if (pthread_create(&tid[t], NULL, executa_tarefa, (void *) t)) {
            printf("--ERRO: pthread_create()\n"); exit(-1);
        }
    }

    //--espera todas as threads terminarem
    for (int t = 0; t < nthreads + 1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n"); exit(-1);
        }
    }

    //--finaliza mutex e variáveis condicionais
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_multiplo);
    pthread_cond_destroy(&cond_impresso);

    printf("Valor final de 'soma' = %ld\n", soma);
    return 0;
}