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
    int *id = (int *) arg;
    printf("Sou a thread produtora %d\n", *id);
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

void *consumidora(void *arg) {
    int *id = (int *) arg;
    printf("Sou a thread consumidora %d\n", *id);
}

int main (int argc, char *argv[]) {

     if(argc < 3) { 
        fprintf(stderr, "Digite: %s <num de threads consumidoras/escritoras> <tamanho do bloco> \n", argv[0]);
        return 1;
    }

    C = atoi(argv[1]);
    N = atoi(argv[2]);

    pthread_t threads[C + 1];
    int *id[C];
    int i;

    //aloca espaço para os ids das threads
    for(i = 0; i < C; i++) {
        id[i] = malloc(sizeof(int));
        if (id[i] == NULL){
            exit(-1);
        }
        *id[i] = i+1;
    }

    //aloca espaço para o buffer
    buffer = ( int *) malloc(sizeof(int) * N);

    //inicializa o buffer
    inicializeBuffer(N);

    // Inicializa os semáforos
    sem_init(&fullSlot, 0, 0);
    sem_init(&emptySlot, 0, N);
    sem_init(&bufferCheio, 0, 0);

    //criando threads produtoras
    if(pthread_create(&threads[0], NULL, produtora, (void *) 0)){
        exit(-1);
    }

    //criando threads consumidoras / escritoras
    for(i = 1; i <= C; i++){
        if(pthread_create(&threads[i], NULL, consumidora, (void *) id[i])){
            exit(-1);
        }
    }

    return 0;
}