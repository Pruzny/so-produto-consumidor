#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include </usr/include/semaphore.h>
#include <string.h>

#define BUFF_SIZE   5		/* total number of slots */
#define NP          3		/* total number of producers */
#define NCP         3		/* total number of consumers/producers */
#define NC          3		/* total number of consumers */
#define NITERS      4		/* number of items produced/consumed */
#define PATH "files/"       /* Diretório dos arquivos de entrada.in */
#define N 5                 /* Quantidade de arquivos */
#define ORDER_MATRIX 10     /**/
#define ORDER_VETOR 10


typedef struct  {
    char nome[50];
    double **matrizA;
    double **matrizB;
    double **matrizC;
    double *vetor;
    double E;

} S;


typedef struct {
    S* buf[BUFF_SIZE];   /* shared var */
    int in;         	  /* buf[in%BUFF_SIZE] is the first empty slot */
    int out;        	  /* buf[out%BUFF_SIZE] is the first full slot */
    sem_t full;     	  /* keep track of the number of full spots */
    sem_t empty;    	  /* keep track of the number of empty spots */
    sem_t mutex;    	  /* enforce mutual exclusion to shared data */
    int teste;
} sbuf_t;

sbuf_t shared[3];

typedef struct {
    double **matrizA;
    double **matrizB;
} matrizes;

void printMatrix(double **a, int order_matrix) {
   for (int i = 0; i < order_matrix; i++) {
        for (int j = 0; j < order_matrix; j++) {
            printf("%lf ", a[i][j]);
        }
        printf("\n");
   }
   printf("\n");
}

double** alloc_matrix(int number_rows, int number_collumns) {
    double **matrix;
    matrix = (double **) malloc(number_rows * sizeof(double *));
    for(int i = 0; i < number_rows; i++) {
        matrix[i] = (double *)malloc(number_collumns * sizeof(double));
    }
    return matrix;
}

double* alloc_vetor(int n) {
    double *v =  (double *)malloc(n * sizeof(double));
    for(int i = 0; i < n; i++) {
        v[i] = 0;
    }
}

//buffer tamanho fixo dinamico
S ** alloc_struct_S(int n, int order_matrix, int order_vetor) {
    S **structure = (S **) malloc(sizeof(S*)*n);
     for (int i= 0; i < n; i++){
        structure[i] = (S *) malloc(sizeof(S));
        structure[i]->matrizA = alloc_matrix(order_matrix, order_matrix);
        structure[i]->matrizB = alloc_matrix(order_matrix, order_matrix);
        structure[i]->matrizC = alloc_matrix(order_matrix, order_matrix);
        structure[i]->vetor = alloc_vetor(order_vetor);
     }

    return structure;
}

matrizes ** alloc_struct_matrizes(int n, int order_matrix) {
    matrizes **matrizes_retornadas = (matrizes **) malloc(sizeof(matrizes*)*n);
    for (int i= 0; i < n; i++){
        matrizes_retornadas[i] = (matrizes *) malloc(sizeof(matrizes));
        matrizes_retornadas[i]->matrizA = alloc_matrix(order_matrix,order_matrix);
        matrizes_retornadas[i]->matrizB = alloc_matrix(order_matrix,order_matrix);
    }
    return matrizes_retornadas;
}

void read_matrix_archive(char *name_archive, double **a, double **b, int order_matrix) {
    FILE *archive_matrix;
    char path_archive_matrix[80];
    strcpy(path_archive_matrix, PATH);
    strcat(path_archive_matrix, name_archive);

    archive_matrix = fopen(path_archive_matrix, "r");
    if (archive_matrix != NULL) {
        double n;
        for (int i = 0; i < order_matrix; i++) {
            for (int j = 0; j < order_matrix; j++) {
                fscanf(archive_matrix, "%lf ", &n);
                a[i][j] = n;
            }
        }
        fscanf(archive_matrix, "\n", &n);
        for (int i = 0; i < order_matrix; i++) {
            for (int j = 0; j < order_matrix; j++) {
                fscanf(archive_matrix, "%lf ", &n);
                b[i][j] = n;
            }
        }
        fclose(archive_matrix);
    } else {
         printf("Houve um problema ao abrir o arquivo %s, verifique o diretório\n", name_archive);
    }
}

matrizes **alloc_matrix_from_archive(char *name_archive, int order_matrix) {
    matrizes **matrizes1 = alloc_struct_matrizes(1, order_matrix);
    read_matrix_archive(name_archive, matrizes1[0]->matrizA, matrizes1[0]->matrizB, order_matrix);
    return matrizes1;
}

matrizes ** read_entrada_in(S **estrutura, char *name_archive, int order_matrix) {
    FILE *archive;
    int count = 0;
    char path_archive[80];
    matrizes **matrizes_retorno;
    strcpy(path_archive, PATH);
    archive = fopen(strcat(path_archive, name_archive), "r");
    if (archive != NULL) {
        char str[100];
        while (!feof(archive)) {
            fscanf(archive, "%s\n", str);
            matrizes_retorno = alloc_matrix_from_archive(str, order_matrix);
            estrutura[count]->matrizA = matrizes_retorno[0]->matrizA;
            estrutura[count]->matrizB = matrizes_retorno[0]->matrizB;
            count++;
        }
        fclose(archive);
    } else {
        printf("Houve um problema ao abrir o arquivo %s, verifique o diretório\n", name_archive);
    }
    return estrutura;
}

double** multiplica_matriz(double **a, double **b, int order_matrix) {
    double **c = alloc_matrix(order_matrix, order_matrix);

    for (int i = 0; i < order_matrix; i++) {
        for (int j = 0; j < order_matrix; j++) {
            c[i][j] = 0;
            for (int k = 0; k < order_matrix; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return c;
}

void *Producer(void *arg) {
    int index = *((int *)arg);
    S *estrutura = (S*)malloc(sizeof(S));
    FILE *archive;
    matrizes **matrizes_retorno;
    int count = 0;
    char path_archive[80];
    strcpy(path_archive, PATH);
    archive = fopen(strcat(path_archive, "entrada.in"), "r");

        /* Produce item */
        if (archive != NULL) {
            char str[100];
            while (!feof(archive)) {
                fscanf(archive, "%s\n", str);
                printf("Thread TID: %d\n", index);
                matrizes_retorno = alloc_matrix_from_archive(str, ORDER_MATRIX);
                estrutura->matrizA = matrizes_retorno[0]->matrizA;
                estrutura->matrizB = matrizes_retorno[0]->matrizB;

                 /* If there are no empty slots, wait */
                sem_wait(&shared[0].empty);
                /* If another thread uses the buffer, wait */
                sem_wait(&shared[0].mutex);
                shared[0].buf[shared[0].in] = estrutura;
                shared[0].in = (shared[0].in+1)%BUFF_SIZE;
                fflush(stdout);
                /* Release the buffer */
                sem_post(&shared[0].mutex);
                /* Increment the number of full slots */
                sem_post(&shared[0].full);
                count++;
            }

            fclose(archive);
        }
        /* Prepare to write item to buf */

    return NULL;
}

void *ConsumerProducer(void *arg) {
    int i, item, index;
    S *estrutura;
    index = *((int *)arg);

    for (i=0; i < NITERS; i++) {

        //A partir daqui: lê do buffer compartilhado 0 *consome
	    /* Prepare to read item from buf */
        /* If there are no filled slots, wait */
        sem_wait(&shared[0].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[0].mutex);
        estrutura = shared[0].buf[shared[0].out];
        shared[0].out = (shared[0].out+1)%BUFF_SIZE;
        printf("[CP1_%d] Consuming ...\n", index);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[0].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[0].empty);

        //multiplica as matrizes
        estrutura->matrizC = multiplica_matriz(estrutura->matrizA, estrutura->matrizB, ORDER_MATRIX);

        //A partir daqui: coloca no buffer compartilhado 1 produz
        /* Prepare to write item to buf */
        /* If there are no empty slots, wait */
        sem_wait(&shared[1].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[1].mutex);
        shared[1].buf[shared[1].in] = estrutura;
        shared[1].in = (shared[1].in+1)%BUFF_SIZE;
        printf("[CP1_%d] Producing...\n", index);
        fflush(stdout);
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
        printf("[C_%d] Consuming %d ...\n", index, item);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[1].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[1].empty);

    }
    return NULL;
}

int main() {
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
