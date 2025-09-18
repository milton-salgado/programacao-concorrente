#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *imprime_id(void *arg) {
    long int id_thread = (long int) arg;
    printf("ID da thread: %ld\n", id_thread);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    long int num_tid;

    if (argc < 2) {
        printf("--Parametro num_tid nao foi recebido.\n");
        return 1;
    }
    num_tid = atoi(argv[1]);

    pthread_t tid[num_tid];

    for (long int i = 0; i < num_tid; i++) {
        printf("--Criando a thread %ld...\n", i + 1);
        if (pthread_create(&tid[i], NULL, imprime_id, (void *) i + 1)) {
            printf("--Erro na criacao da thread %ld com pthread_create().\n", i + 1);
            return 2;
        }
    }

    printf("--Thread principal terminou.\n");
    pthread_exit(NULL);
}