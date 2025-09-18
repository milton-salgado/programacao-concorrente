#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 5
#define PASSOS 4

pthread_mutex_t mutex;
pthread_cond_t cond;

void barreira(int num_threads) {
    static int bloqueadas = 0;

    pthread_mutex_lock(&mutex);
    if (bloqueadas == (num_threads - 1)) {
        pthread_cond_broadcast(&cond);
        bloqueadas = 0;
    } else {
        bloqueadas++;
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void *A(void *t) {
    int meu_id = *(int *) t;
    int boba1, boba2;

    for (long int i = 0; i < PASSOS; i++) {
        printf("--LOG: Thread %d: passo=%ld\n", meu_id, i);
        barreira(NUM_THREADS);

        // ? processamento qualquer
        boba1 = 100; boba2 = -100;
        while (boba2 < boba1)
            boba2++;
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t tid[NUM_THREADS];
    int id[NUM_THREADS];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for (long i = 0; i < NUM_THREADS; i++) {
        id[i] = i;
        pthread_create(&tid[i], NULL, A, (void *) &id[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(tid[i], NULL);

    printf("--LOG: FIM\n");

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}