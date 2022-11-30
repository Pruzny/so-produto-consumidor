#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include </usr/include/semaphore.h>

#define BUFF_SIZE   5		/* total number of slots */
#define NP          3		/* total number of producers */
#define NCP         3		/* total number of consumers/producers */
#define NC          3		/* total number of consumers */
#define NITERS      4		/* number of items produced/consumed */

typedef struct {
    int buf[BUFF_SIZE];   /* shared var */
    int in;         	  /* buf[in%BUFF_SIZE] is the first empty slot */
    int out;        	  /* buf[out%BUFF_SIZE] is the first full slot */
    sem_t full;     	  /* keep track of the number of full spots */
    sem_t empty;    	  /* keep track of the number of empty spots */
    sem_t mutex;    	  /* enforce mutual exclusion to shared data */
} sbuf_t;

sbuf_t shared[3];


typedef struct  {
    char nome[50];
    int** matrizA;
    int** matrizB;
    int** matrizC;
    int* vetor;
    double E;

} S;

void read_matrix_archive(char *name_archive, float **a, int order_matrix) {
    FILE *archive;
    archive = fopen(name_archive, "r");
    if (archive != NULL) {
        float n;
        while (!feof(archive)) {
            for (int i = 0; i < order_matrix; i++) {
                for (int j = 0; j < order_matrix; j++) {
                    fscanf(archive, "%f ", &n);
                    a[i][j] = n;
                }
            }
        }
        fclose(archive);
    }
}


void printMatrix(float **a, int order_matrix) {
   for (int i = 0; i < order_matrix; i++) {
        for (int j = 0; j < order_matrix; j++) {
            printf("%f ", a[i][j]);
        }
        printf("\n");
   }
   printf("\n");
}

float** alloc_matrix(int number_rows, int number_collumns) {
    float **matrix;
    matrix = (float **) malloc(number_rows * sizeof(float *));
    for(int i = 0; i < number_rows; i++) {
        matrix[i] = (float *)malloc(number_collumns * sizeof(float));
    }
    return matrix;
}

float* alloc_vetor(int n) {
    float *v =  (float *)malloc(n * sizeof(float));
    for(int i = 0; i < n; i++) {
        v[i] = 0;
    }
}

float** multiplica_matriz(float **a, float **b, int order_matrix) {
    float **c = alloc_matrix(order_matrix, order_matrix);

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            c[i][j] = 0;
            for (int k = 0; k < 10; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return c;
}

/*P - Thread produtora - Le um arquivo (entrada.in) contendo uma lista de 50 arquivos
de entrada (um nome de arquivo por linha), cada um contendo duas matrizes quadradas
de ordem 10 de doubles.
 A cada arquivo lida, a thread produtora cria dinamicamente uma estrutura S, preenche o nome do arquivo de entrada, alem de A e B e coloca o ponteiro para
 a estrutura S em shared[0].buffer[in] para ser processada pela etapa seguinte. S ́o teremos 1
instancia desta thread.
 */

void *Producer(void *arg) {
    int i, item, index;

    index = *((int *)arg);

    for (i=0; i < NITERS; i++) {

        /* Produce item */
        item = i+(index*1000);

        /* Prepare to write item to buf */

        /* If there are no empty slots, wait */
        sem_wait(&shared[0].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[0].mutex);
        shared[0].buf[shared[0].in] = item;
        shared[0].in = (shared[0].in+1)%BUFF_SIZE;
        printf("[P_%d] Producing %d ...\n", index, item);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[0].mutex);
        /* Increment the number of full slots */
        sem_post(&shared[0].full);
    }
    return NULL;
}

void *ConsumerProducer(void *arg)
{
    int i, item, index;

    index = *((int *)arg);

    for (i=0; i < NITERS; i++) {

	/* Prepare to read item from buf */

        /* If there are no filled slots, wait */
        sem_wait(&shared[0].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[0].mutex);
        item = shared[0].buf[shared[0].out];
        shared[0].out = (shared[0].out+1)%BUFF_SIZE;
        printf("[CP1_%d] Consuming %d ...\n", index, item); fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[0].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[0].empty);

	//Change consumed item
	item += 500;

        /* Prepare to write item to buf */

        /* If there are no empty slots, wait */
        sem_wait(&shared[1].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[1].mutex);
        shared[1].buf[shared[1].in] = item;
        shared[1].in = (shared[1].in+1)%BUFF_SIZE;
        printf("[CP1_%d] Producing %d ...\n", index, item); fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[1].mutex);
        /* Increment the number of full slots */
        sem_post(&shared[1].full);

    }
    return NULL;
}

void *Consumer(void *arg)
{
    int i, item, index;

    index = *((int *)arg);

    for (i=0; i < NITERS; i++) {

        /* Prepare to read item from buf */

        /* If there are no filled slots, wait */
        sem_wait(&shared[1].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[1].mutex);
        item = shared[1].buf[shared[1].out];
        shared[1].out = (shared[1].out+1)%BUFF_SIZE;
        printf("[C_%d] Consuming %d ...\n", index, item); fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[1].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[1].empty);

    }
    return NULL;
}

int main()
{
    float **a = alloc_matrix(10, 10);
    read_matrix_archive("matriz0.txt", a, 10);
    printMatrix(a, 10);

    float **b = alloc_matrix(10, 10);
    read_matrix_archive("matriz1.txt", b, 10);
    printMatrix(b, 10);

    float **c = multiplica_matriz(a, b, 10);
    printMatrix(c, 10);

    pthread_t idP, idC, idCP;
    int index;
    int sP[NP], sC[NC], sCP[NCP];

    for (index=0; index<2; index++) {
    	sem_init(&shared[index].full, 0, 0);
    	sem_init(&shared[index].empty, 0, BUFF_SIZE);
    	sem_init(&shared[index].mutex, 0, 1);
    }

    for (index = 0; index < NP; index++)
    {
       sP[index]=index;
       /* Create a new producer */
       pthread_create(&idP, NULL, Producer, &sP[index]);
    }

    for (index = 0; index < NCP; index++)
    {
       sCP[index]=index;
       /* Create a new producer */
       pthread_create(&idCP, NULL, ConsumerProducer, &sCP[index]);
    }

    for (index = 0; index < NC; index++)
    {
       sC[index]=index;
       /* Create a new consumer */
       pthread_create(&idC, NULL, Consumer, &sC[index]);
    }

    pthread_exit(NULL);
}
