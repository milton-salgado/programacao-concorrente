#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

long int soma = 0;

void *executa_tarefa(void *arg) {
    long int id = (long int) arg;
    printf("--LOG: Thread : %ld esta executando...\n", id);

    for (int i = 0; i < 100000; i++)
        soma++;

    printf("--LOG: Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t *tid;
    int num_threads;

    if (argc < 2) {
        printf("--ERRO: Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    num_threads = atoi(argv[1]);

    if (!(tid = (pthread_t *) calloc(num_threads, sizeof(pthread_t)))) {
        printf("--ERRO: malloc");
        return 2;
    }

    for (long int t = 0; t < num_threads; t++)
        if (pthread_create(&tid[t], NULL, executa_tarefa, (void *) t)) {
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }

    for (int t = 0; t < num_threads; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1);
        }
    }

    printf("--LOG: Valor de 'soma' = %ld\n", soma);

}