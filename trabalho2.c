#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define P 1

//Variáveis globais
int C;
int N;
int *buffer;
sem_t fullSlot, emptySlot, bufferCheio;

void inicializeBuffer(int n) {
    int i;
    for(i = 0; i < n; i++) {
        buffer[i] = 0;
    }  
}

void imprimeBuffer(int n) {
  printf("--- Buffer ");
  for (int i = 0; i < n; i ++)
    printf("%d ", buffer[i]);
  printf(" ---\n\n");
}

void *produtora(void *arg) {
    long int id = (long int) arg;
    printf("Sou a thread produtora %ld\n", id);
    static int in = 0;
    //lê o arquivo
    //....
    //preechimento do buffer
    while (1) {
        sem_wait(&emptySlot);
        buffer[in] = rand() % 10;
        in = (in + 1) % N;
        sem_post(&fullSlot);
        if (in == (N - 1)) {
            printf("\n--- Buffer esta cheio ---\n");
            imprimeBuffer(5);
            sem_post(&bufferCheio);
        }
        sleep(1);
    }
    imprimeBuffer(5);
}

void *consumidora (void *arg) {
    long int id = (long int) arg;
    printf("Sou a thread consumidora %ld\n", id);
    static int out = 0;
    while (1) {
        printf("Thread %ld chegou no while\n", id);
        sleep(5);
    }
    imprimeBuffer(id);
}

int main (int argc, char *argv[]) {
    pthread_t *threads;
    long int i;

     if(argc < 3) { 
        fprintf(stderr, "Digite: %s <num de threads consumidoras/escritoras> <tamanho do bloco> \n", argv[0]);
        return 1;
    }

    C = atoi(argv[1]);
    N = atoi(argv[2]);

    //aloca espaço para o buffer
    buffer = ( int *) malloc(sizeof(int) * N);
    if(buffer == NULL) {
        fprintf(stderr, "ERRO: Um erro ocorreu durante o malloc\n");
        return 2;
    }

    //inicializa o buffer
    inicializeBuffer(N);

    //aloca espaço para identificados das threads
    threads = (pthread_t *) malloc(sizeof(pthread_t) * (C + 1));
    if (threads == NULL){
        fprintf(stderr, "ERRO: Um erro ocorreu durante o malloc\n");
        return 2;
    }

    // Inicializa os semáforos
    sem_init(&fullSlot, 0, 0);
    sem_init(&emptySlot, 0, N);
    sem_init(&bufferCheio, 0, 0);

    //criando threads produtoras
    if(pthread_create(threads + 0, NULL, produtora, (void *) 0)){
        exit(-1);
    }

    //criando threads consumidoras / escritoras
    for(i = 1; i < (C + 1); i++){
        if(pthread_create(threads + i, NULL, consumidora, (void *) i)){
            exit(-1);
        }
    }

    free(buffer);
    free(threads);

    return 0;
}