#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

long int cont_primos = 0, num_elementos = 0, numeroAtual = 1;
pthread_mutex_t mutex;

int ehPrimo(long long int n) {
    int i;

    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (i = 3; i < sqrt(n) + 1; i += 2)
        if (n % i == 0)
            return 0;

    return 1;
}

void *conta_primos(void *args) {
    long int aux = 0;

    while (1) {
        pthread_mutex_lock(&mutex);
        aux = numeroAtual++;
        pthread_mutex_unlock(&mutex);

        if (aux > num_elementos)
            break;

        if (ehPrimo(aux)) {
            printf("--LOG: Primo encontrado: %ld\n", aux);
            pthread_mutex_lock(&mutex);
            cont_primos++;
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    long int num_threads = 0;
    pthread_t *tid = NULL;

    if (argc < 3) {
        printf("--ERRO: Faltam argumentos.\n");
        return 1;
    }

    num_elementos = atoll(argv[1]);
    num_threads = atoll(argv[2]);

    if (!(tid = (pthread_t *) calloc(num_threads, sizeof(pthread_t)))) {
        printf("--ERRO: calloc()");
        return 2;
    }

    pthread_mutex_init(&mutex, NULL);

    for (long int i = 0; i < num_threads; i++) {

        if (pthread_create(&tid[i], NULL, conta_primos, NULL)) {
            printf("--ERRO: pthread_create()");
            return 4;
        }
    }

    for (long int i = 0; i < num_threads; i++) {
        if (pthread_join(tid[i], NULL)) {
            printf("--ERRO: pthread_join()");
            return 4;
        }
    }

    pthread_mutex_destroy(&mutex);
    printf("--LOG: Total de primos da sequencia: %ld\n", cont_primos);

    free(tid);

    return 0;
}