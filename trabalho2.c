#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define P 1

//Variáveis globais
int C;
int N;
int *buffer[10];
char *filename_input, *filename_output;
int num_elements;
sem_t fullSlot, emptySlot;
sem_t mutexCons, mutexEscr;

void inicializeBuffer(int n) {
    int i, j;
    for(j = 0; j < 10; j++) {
        for(i = 0; i < n; i++) {
            buffer[j][i] = 0;
        }  
    }
}

void imprimeBuffer(int n) {
  printf("--- Buffer ");
  for(int j = 0; j < 10; j++) {
        for(int i = 0; i < n; i++) {
           printf("%d ", buffer[j][i]);
        }  
  }
  printf(" ---\n\n");
}

void *produtora(void *arg) {
    printf("Thread produtora começou\n");
    FILE * input = (FILE *) arg;
    static int in = 0;
    int count = 0;

    //lê o arquivo e preenche o buffer
    while(count < (num_elements / N)) {
        sem_wait(&emptySlot);
        printf("Thread produtora entrou na secao critica\n");
        for(int i = 0; i < N; i++) {
            fscanf(input, "%d", &buffer[in][i]);
        }

        in = (in + 1) % 10;
        sem_post(&fullSlot);
        count++;
    }
    printf("Thread produtora terminou\n");
    pthread_exit(NULL);
}

void *consumidora (void *arg) {
    printf("Thread consumidora começou\n");
    static int out = 0;
    int count = 0;
    FILE * output = (FILE *) arg;
    int local_buffer[N];

    while (count < (num_elements / N)) 
    {
        sem_wait(&fullSlot);
        sem_wait(&mutexCons);
        
        for(int i = 0; i < N; i++) {
            local_buffer[i] = buffer[out][i];
        }

        out = (out + 1) % 10;

        sem_post(&mutexCons);
        sem_post(&emptySlot);

        for(int i = 1; i < N; i++) {
            for(int j = 0; j < N - 1; j++) {
                if(local_buffer[j] < local_buffer[j - 1]) {
                    int aux = local_buffer[j];
                    local_buffer[j] = local_buffer[j + 1];
                    local_buffer[j + 1] = aux;
                }
            }
        }

        sem_wait(&mutexEscr);

        for(int i = 0; i < N; i++) {
            fprintf(output, "%d ", local_buffer[i]);
        }

        fprintf(output, "\n");

        sem_post(&mutexEscr);
    }
    
}

int main (int argc, char *argv[]) {
    pthread_t *threads;
    long int i;
    FILE *entrada, *saida;

     if(argc < 5) { 
        fprintf(stderr, "Digite: %s <num de threads consumidoras/escritoras> <tamanho do bloco> <caminho do arquivo de entrada> <caminho do arquivo de saida>\n", argv[0]);
        return 1;
    }

    C = atoi(argv[1]);
    N = atoi(argv[2]);
    filename_input = argv[3];
    filename_output = argv[4];

    //aloca espaço para o buffer
    for(int i = 0; i < 10; i ++) {
        buffer[i] = ( int *) malloc(sizeof(int) * N);
        if(buffer == NULL) {
            fprintf(stderr, "ERRO: Um erro ocorreu durante o malloc\n");
            return 2;
        }
    }

    //inicializa o buffer
    inicializeBuffer(N);

    //aloca espaço para identificador das threads
    threads = (pthread_t *) malloc(sizeof(pthread_t) * (C + 1));
    if (threads == NULL){
        fprintf(stderr, "ERRO: Um erro ocorreu durante o malloc\n");
        return 2;
    }


    // Inicializa os semáforos
    sem_init(&fullSlot, 0, 0);
    sem_init(&emptySlot, 0, N);
    sem_init(&mutexCons, 0, 1);
    sem_init(&mutexEscr, 0, 0);

    //obtém o número de elementos no arquivo
    entrada = fopen(filename_input, "r");
    if(entrada == NULL) {
        fprintf(stderr, "ERRO: Um erro ocorreu durante a leitura do arquivo\n");
        exit(-1);
    } else {
        fscanf(entrada, "%d\n", &num_elements);
    }

    if((num_elements % 10) != 0) {
        fprintf(stderr, "ERRO: Arquivo de entrada deve conter um número de elementos múltiplo de 10\n");
        exit(-1);
    }

    //abre arquivo de saída
    saida = fopen(filename_output, "w");
    if (saida == NULL) {
        fprintf(stderr, "ERRO: Um erro ocorreu durante a leitura do arquivo\n");
        exit(-1);
    }

    //criando threads produtoras
    if(pthread_create(threads + 0, NULL, produtora, (void *) entrada)){
        exit(-1);
    }

    //criando threads consumidoras / escritoras
    for(i = 1; i < (C + 1); i++){
        if(pthread_create(threads + i, NULL, consumidora, (void *) saida)){
            exit(-1);
        }
    }

    //espera todas as threads finalizarem
    for(long int i = 0; i < (C + 1); i++) {
        if(pthread_join(*(threads + i), NULL)) {
            exit(-1);
        }
    }

    imprimeBuffer(N);


    free(threads);

    return 0;
}