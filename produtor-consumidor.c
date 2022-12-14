#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include </usr/include/semaphore.h>
#include <string.h>

#define BUFF_SIZE   5		/* total number of slots */
#define NP          1		/* total number of producers */
#define NCP1        5		/* total number of consumers/producers CP1*/
#define NCP2        4		/* total number of consumers/producers CP1*/
#define NCP3        3		/* total number of consumers/producers CP1*/
#define NC          1		/* total number of consumers */
#define PATH "files/"       /* Diretório dos arquivos de entrada.in */
#define N 50                /* Quantidade de arquivos */
#define ORDER_MATRIX 10     /**/
#define ORDER_VECTOR 10


typedef struct  {
    char filename[100];
    double **matrixA;
    double **matrixB;
    double **matrixC;
    double *vector;
    double E;

} S;


typedef struct {
    S* buf[BUFF_SIZE];   /* shared var */
    int in;         	  /* buf[in%BUFF_SIZE] is the first empty slot */
    int out;        	  /* buf[out%BUFF_SIZE] is the first full slot */
    sem_t full;     	  /* keep track of the number of full spots */
    sem_t empty;    	  /* keep track of the number of empty spots */
    sem_t mutex;    	  /* enforce mutual exclusion to shared data */
} sbuf_t;

sbuf_t shared[4];

typedef struct {
    double **matrixA;
    double **matrixB;
} arrays;

double** alloc_matrix(int number_rows, int number_collumns) {
    double **matrix;
    matrix = (double **) malloc(number_rows * sizeof(double *));
    for(int i = 0; i < number_rows; i++) {
        matrix[i] = (double *)malloc(number_collumns * sizeof(double));
    }
    return matrix;
}

double* alloc_vector(int order_vector) {
    double *v =  (double *)malloc(order_vector * sizeof(double));
    for(int i = 0; i < order_vector; i++) {
        v[i] = 0;
    }
    return v;
}

arrays ** alloc_struct_arrays(int n, int order_matrix) {
    arrays **arrays_retornadas = (arrays **) malloc(sizeof(arrays*)*n);
    for (int i= 0; i < n; i++){
        arrays_retornadas[i] = (arrays *) malloc(sizeof(arrays));
        arrays_retornadas[i]->matrixA = alloc_matrix(order_matrix,order_matrix);
        arrays_retornadas[i]->matrixB = alloc_matrix(order_matrix,order_matrix);
    }
    return arrays_retornadas;
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
        fscanf(archive_matrix, "\n");
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

arrays **alloc_matrix_from_archive(char *name_archive, int order_matrix) {
    arrays **arrays1 = alloc_struct_arrays(1, order_matrix);
    read_matrix_archive(name_archive, arrays1[0]->matrixA, arrays1[0]->matrixB, order_matrix);
    return arrays1;
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

double * sum_col_matrix(double **matrix, int order_matrix) {
    double * vector = alloc_vector(ORDER_VECTOR);
    for (int i = 0; i < order_matrix; i++)  {
        double sum = 0.0;
        for (int j = 0; j < order_matrix; j++)  {
            sum += matrix[j][i];
        }
        vector[i] = sum;
    }
    return vector;
}

double sum_el_vector(double * vector, int order_vector) {
    double sum = 0;
    for (int i=0; i < order_vector; i++) {
        sum += vector[i];
    }
    return sum;
}

void *Producer(void *arg) {
    int index = *((int *)arg);
    //Criando o ponteiro para o arquivo
    FILE *archive;
    //Criando o ponteiro de ponteiro para alocar as arrays que serão retornadas
    arrays **arrays_temp;
    int count = 0;
    //Para o Path depender do define PATH
    char path_archive[80];
    strcpy(path_archive, PATH);
    //Abrindo o arquivo
    archive = fopen(strcat(path_archive, "entrada.in"), "r");

    //Se o arquivo for aberto corretamente
    if (archive != NULL) {
        //pegando a string do entrada.in
        char str[100];
        //enquanto o arquivo não acabar
        while (!feof(archive)) {
            //alocando a estrutura S
            S *estrutura = (S*)malloc(sizeof(S));
            fscanf(archive, "%s\n", str);

            //Receber uma matriz baseado no matrizN.in que foi lido e colocado em str
            arrays_temp = alloc_matrix_from_archive(str, ORDER_MATRIX);
            //A estrutura recebe a matriz A e a matriz B
            estrutura->matrixA = arrays_temp[0]->matrixA;
            estrutura->matrixB = arrays_temp[0]->matrixB;
            //prenche o nome do arquivo de entrada
            strcpy(estrutura->filename, str);
            printf("[P_%d]: Nome do Arquivo %s TID: %d\n", index, estrutura->filename, index);
            /* Buffer compartilhado*/
             /* If there are no empty slots, wait */
            sem_wait(&shared[0].empty);
            /* If another thread uses the buffer, wait */
            sem_wait(&shared[0].mutex);
            //Colocando a matriz no buffer compartilhado
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
    return NULL;
}

void *ConsumerProducer(void *arg) {
    int i, index;
    S *estrutura;
    index = *((int *)arg);

    for (i=0; i < N; i++) {

        //Lê o buffer compartilhado
        /* If there are no filled slots, wait */
        sem_wait(&shared[0].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[0].mutex);
        estrutura = shared[0].buf[shared[0].out];
        shared[0].out = (shared[0].out+1)%BUFF_SIZE;
        printf("[CP1_%d] Lendo %s do buffer compartilhado...\n", index, estrutura->filename);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[0].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[0].empty);

        //multiplica as arrays
        estrutura->matrixC = multiplica_matriz(estrutura->matrixA, estrutura->matrixB, ORDER_MATRIX);
        //Coloca estrutura no buffer compartilhado
        /* If there are no empty slots, wait */
        sem_wait(&shared[1].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[1].mutex);
        shared[1].buf[shared[1].in] = estrutura;
        shared[1].in = (shared[1].in+1)%BUFF_SIZE;
        printf("[CP1_%d] Escrevendo %s no Buffer compartilhado...\n", index,  estrutura->filename);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[1].mutex);
        /* Increment the number of full slots */
        sem_post(&shared[1].full);
    }
    fflush(stdout);
    return NULL;
}

void *ConsumerProducerCP2(void *arg) {
    int i, index;
    S *estrutura;
    double *vector;
    index = *((int *)arg);
    for (i=0; i < N; i++) {

        //Lê o buffer compartilhado
        /* If there are no filled slots, wait */
        sem_wait(&shared[1].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[1].mutex);
        estrutura = shared[1].buf[shared[1].out];
        shared[1].out = (shared[1].out+1)%BUFF_SIZE;
        printf("[CP2_%d] Lendo %s do buffer compartilhado\n", index, estrutura->filename);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[1].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[1].empty);

        //Somar a coluna de Matriz C
        vector = sum_col_matrix(estrutura->matrixC, ORDER_MATRIX);
        //Coloca o vector V na estrutura temporária
        estrutura->vector = vector;

        //Coloca estrutura no buffer compartilhado
        /* If there are no empty slots, wait */
        sem_wait(&shared[2].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[2].mutex);
        shared[2].buf[shared[2].in] = estrutura;
        shared[2].in = (shared[2].in+1)%BUFF_SIZE;
        printf("[CP2_%d] Colocando %s no Buffer compartilhado...\n", index, estrutura->filename);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[2].mutex);
        /* Increment the number of full slots */
        sem_post(&shared[2].full);

    }
    fflush(stdout);
    return NULL;
}

void *ConsumerProducerCP3(void *arg) {
    int i, index;
    S *estrutura;
    index = *((int *)arg);
    for (i=0; i < N; i++) {

        //Lê o buffer compartilhado
        /* If there are no filled slots, wait */
        sem_wait(&shared[2].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[2].mutex);
        estrutura = shared[2].buf[shared[2].out];
        shared[2].out = (shared[2].out+1)%BUFF_SIZE;
        printf("[CP3_%d] Lendo %s do buffer compartilhado\n", index, estrutura->filename);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[2].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[2].empty);

        //Somar elementos do vector
        estrutura->E = sum_el_vector(estrutura->vector, ORDER_VECTOR);

        //Coloca estrutura no buffer compartilhado
        /* If there are no empty slots, wait */
        sem_wait(&shared[3].empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[3].mutex);
        shared[3].buf[shared[3].in] = estrutura;
        shared[3].in = (shared[3].in+1)%BUFF_SIZE;
        printf("[CP3_%d] Colocando %s no Buffer compartilhado...\n", index, estrutura->filename);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[3].mutex);
        /* Increment the number of full slots */
        sem_post(&shared[3].full);
    }
    fflush(stdout);
    return NULL;
}

void *Consumer(void *arg) {
    int i, index;
    S* item;
    index = *((int *)arg);
    FILE *out = fopen("saida.out", "w");

    for (i=0; i < N; i++) {
        /* Prepare to read item from buf */
        /* If there are no filled slots, wait */
        sem_wait(&shared[3].full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared[3].mutex);
        item = shared[3].buf[shared[3].out];
        shared[3].out = (shared[3].out+1)%BUFF_SIZE;
        printf("[C_%d] Pegando a Estrutura para escrever no arquivo ...\n", index);
        fflush(stdout);
        /* Release the buffer */
        sem_post(&shared[3].mutex);
        /* Increment the number of empty slots */
        sem_post(&shared[3].empty);

        //escrever no arquivo saida.out
        fprintf(out, "================================\n");
        fprintf(out, "Entrada: %s\n", item->filename);
        fprintf(out, "——————————–\n");
        for (int ii = 0; ii < ORDER_MATRIX; ii++) {
            for (int jj = 0; jj < ORDER_MATRIX; jj++) {
                fprintf(out, "%lf ", item->matrixA[ii][jj]);
            }
            fprintf(out, "\n");
        }
        fprintf(out, "——————————–\n");
        for (int ii = 0; ii < ORDER_MATRIX; ii++) {
            for (int jj = 0; jj < ORDER_MATRIX; jj++) {
                fprintf(out, "%lf ", item->matrixB[ii][jj]);
            }
            fprintf(out, "\n");
        }
        fprintf(out, "——————————–\n");
        for (int ii = 0; ii < ORDER_MATRIX; ii++) {
            for (int jj = 0; jj < ORDER_MATRIX; jj++) {
                fprintf(out, "%lf ", item->matrixC[ii][jj]);
            }
            fprintf(out, "\n");
        }
        fprintf(out, "——————————–\n");
        for (int ii = 0; ii < ORDER_MATRIX; ii++) {
            fprintf(out, "%lf\n", item->vector[ii]);
        }
        fprintf(out, "——————————–\n");
        fprintf(out, "%lf\n", item->E);
        fprintf(out, "================================\n");
    }
    fclose(out);

    return NULL;
}

int main() {
    pthread_t idP[NP], idC[NC], idCP1[NCP1], idCP2[NCP2], idCP3[NCP3];
    int index;
    int sP[NP], sC[NC], sCP1[NCP1], sCP2[NCP2], sCP3[NCP3];

    for (index=0; index < 4; index++) {
    	sem_init(&shared[index].full, 0, 0);
    	sem_init(&shared[index].empty, 0, BUFF_SIZE);
    	sem_init(&shared[index].mutex, 0, 1);
    }

    for (index = 0; index < NP; index++) {
       sP[index]=index;
       /* Create a new producer */
       pthread_create(&idP[index], NULL, Producer, &sP[index]);
    }

    for (index = 0; index < NCP1; index++) {
       sCP1[index]=index;
       /* Create a new producer */
       pthread_create(&idCP1[index], NULL, ConsumerProducer, &sCP1[index]);
    }

    for (index = 0; index < NCP2; index++) {
       sCP2[index]=index;
       /* Create a new producer */
       pthread_create(&idCP2[index], NULL, ConsumerProducerCP2, &sCP2[index]);
    }

    for (index = 0; index < NCP3; index++) {
       sCP3[index]=index;
       /* Create a new producer */

       pthread_create(&idCP3[index], NULL, ConsumerProducerCP3, &sCP3[index]);
    }

    for (index = 0; index < NC; index++) {
       sC[index]=index;
       /* Create a new consumer */
       fflush(stdout);
       pthread_create(&idC[index], NULL, Consumer, &sC[index]);
       pthread_join(idC[index], NULL);
    }

    for (index = 0; index < NCP1; index++) {
        pthread_cancel(idCP1[index]);
    }

    for (index = 0; index < NCP2; index++) {
        pthread_cancel(idCP2[index]);
    }

    for (index = 0; index < NCP3; index++) {
        pthread_cancel(idCP3[index]);
    }

    pthread_exit(NULL);
}
