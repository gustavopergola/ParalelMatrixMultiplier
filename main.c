#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mpi.h"

#define M1_ROWS_LENGTH 4
#define M1_COLUMNS_LENGTH 6
#define M2_ROWS_LENGTH 6
#define M2_COLUMNS_LENGTH 4

#define ROWS_CHUNK_M2 1

int aborta(char *error_msg){
    printf("%s", error_msg);
    MPI_Abort (MPI_COMM_WORLD, 1);
    return 1;
}

bool isMaster(int rank){
    return rank == 0;
}

bool isSlave(int rank){
    return !isMaster(rank);
}

bool matrizesNaoMultiplicaveis(){
    return M1_COLUMNS_LENGTH != M2_ROWS_LENGTH;
}

void writeMatrix(int rows, int cols, FILE *file){
    int i, j, k;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j){
            k = ++j;
            fwrite(&k, sizeof(int), 1, file);
        }
    }
}

void writeRandomMatrix(int rows, int cols, FILE *file){
    int range = 10;
    int min = 0;
    int random_number;
    int i, j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            random_number = rand() % range + min;
            fwrite(&random_number, sizeof(int), 1, file);
        }
    }
}

void generateNewMatrixFile(int random){
    //1: random matrix  0: not random
    printf("Generating new matrix files\n");
    FILE *fileONE, *fileTWO;
    fileONE = fopen("fileone.bin", "wb+");
    fileTWO = fopen("filetwo.bin", "wb+");

    if(random){
        srand(time(NULL));
        writeRandomMatrix(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, fileONE);
        writeRandomMatrix(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, fileTWO);
    }else{
        writeMatrix(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, fileONE);
        writeMatrix(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, fileTWO);
    }

    fclose(fileONE);
    fclose(fileTWO);
}

void* allocArray (int rows, int cols)
{
    return malloc( sizeof(int[rows][cols]) ); // allocate 1 2D-array
}

void readMatrixFiles(int array1[M1_ROWS_LENGTH][M1_COLUMNS_LENGTH],
                     int array2[M2_ROWS_LENGTH][M2_COLUMNS_LENGTH]){
    //printf("Reading matrix files\n");
    FILE *fileONE, *fileTWO;
    fileONE = fopen("fileone.bin", "rb");
    fileTWO = fopen("filetwo.bin", "rb");
    fread(array1, sizeof(int[M1_ROWS_LENGTH][M1_COLUMNS_LENGTH]), 1, fileONE);
    fread(array2, sizeof(int[M2_ROWS_LENGTH][M2_COLUMNS_LENGTH]), 1, fileTWO);
}

void mostraMatriz(int rows, int cols, int matrix[rows][cols]){
    int i, j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void limpaMatriz(int rows, int cols, int matrix[rows][cols]){
    int i, j;
    for(i = 0; i < rows; i++)
        for(j = 0; j < cols; j++)
            matrix[i][j] = 0;
}

void* matrix_multiplier_sequential(int rowsA, int colsA, int matrixA[rowsA][colsA], int rowsB, int colsB, int matrixB[rowsB][colsB]){
    int (*result)[colsB] = allocArray(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH);

    int i,j,k;
    for (i=0; i<rowsA; i++){
        for (j=0; j<colsB; j++){
            result[i][j] = 0;
            for (k=0; k<rowsB; k++){
                result[i][j] = result[i][j] + matrixA[i][k]*matrixB[k][j];
            }
        }
    }
    return result;
}

void calcula_matriz_resultante_sequencial(int (*matriz_a)[M1_COLUMNS_LENGTH], int (*matriz_b)[M2_COLUMNS_LENGTH]){
    double starttime = 0, endtime = 0;
    starttime = MPI_Wtime();
    int (*resultMatrix)[M2_COLUMNS_LENGTH] = matrix_multiplier_sequential(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, matriz_a, M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, matriz_b);
    endtime = MPI_Wtime();
    printf("Tempo decorrido para o método sequencial: %f\n", endtime-starttime);

    printf("Matriz resultante (SEQUENCIAL):\n");
    mostraMatriz(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH, resultMatrix);
    free(resultMatrix);
}

int main(int argc, char *argv[])
{
    MPI_Init (NULL, NULL);
    int comm_rank = 0;
    int comm_size = 0;
    int method = 0;
    int sequencial = 0;

    MPI_Comm_rank (MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &comm_size);
    MPI_Request isreq, irreq;
    MPI_Status mpi_status;

    int primeiro_chunk_linhas;

    if (comm_size > 1)
         primeiro_chunk_linhas = M1_ROWS_LENGTH   / (comm_size - 1);

    int total_chunk_size_m1 = primeiro_chunk_linhas * M1_COLUMNS_LENGTH;
    int total_chunk_size_mr = primeiro_chunk_linhas * M2_COLUMNS_LENGTH;

    int i;
    for(i=1; i < argc; i++)
        if (strcmp(argv[i], "m1") == 0) method = 1;
        else if (strcmp(argv[i], "m2") == 0) method = 2;

    if (isMaster(comm_rank)){
        printf("Comm size = %d\n", comm_size);

        for(i=1; i < argc; i++){
            if (strcmp(argv[i], "g") == 0) generateNewMatrixFile(0);
            else if (strcmp(argv[i], "g+") == 0) generateNewMatrixFile(1);
            else if (strcmp(argv[i], "s") == 0) sequencial = 1;//random matrix
            else if (strcmp(argv[i], "m1") == 0) printf("Executando método 1\n");
            else if (strcmp(argv[i], "m2") == 0) printf("Executando método 2\n");
        }

        if(matrizesNaoMultiplicaveis()) return aborta("Matrizes não são multiplicáveis!\n");
        int (*matriz_a)[M1_COLUMNS_LENGTH] = allocArray(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH);
        int (*matriz_b)[M2_COLUMNS_LENGTH] = allocArray(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH);
        int (*matriz_resultante)[M2_COLUMNS_LENGTH] = allocArray(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH);

        readMatrixFiles(matriz_a, matriz_b);

        printf("Matriz A:\n");
        mostraMatriz(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, matriz_a);
        printf("Matriz B:\n");
        mostraMatriz(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, matriz_b);

        if (sequencial == 1){
            calcula_matriz_resultante_sequencial(matriz_a, matriz_b);
            printf("calculo de matriz sequencial terminada!\n");
            MPI_Finalize();
            return 0;
        }

        if(comm_size == 1){
            printf("Não é possível utilizar os métodos paralelos com apenas uma thread\ntente utilizar 2 threads.\n");
            MPI_Finalize();
            return 0;
        }

        double starttime = 0, endtime = 0;
        starttime = MPI_Wtime();

        if(method == 1 || method == 0){

            // todo fazer esse numero aqui poder n ser perfeitamente divisivel
            if(M1_ROWS_LENGTH % (comm_size - 1) != 0){
                printf("Número de threads inválido para o método 1\n");
                printf("O número de linhas da primeira matriz deve ser divisível pelo número de threads - 1\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
                return 0;
            }

            for(i = 0; i < comm_size - 1; i)
                MPI_Send(matriz_a[(primeiro_chunk_linhas) * i++], primeiro_chunk_linhas * M1_ROWS_LENGTH, MPI_INT, i, 1, MPI_COMM_WORLD);

            for(i = 0; i < comm_size - 1; i)
                MPI_Send(matriz_b, M2_COLUMNS_LENGTH * M2_ROWS_LENGTH, MPI_INT, ++i, 2, MPI_COMM_WORLD);

            int (*resultado_parcial)[M2_COLUMNS_LENGTH] = allocArray(primeiro_chunk_linhas, M2_COLUMNS_LENGTH);

            int quemEnviou, linhaAtualParcial;

            for(i = 0; i < comm_size -1; i++){
                quemEnviou = i + 1;
                MPI_Recv(resultado_parcial, total_chunk_size_mr, MPI_INT, quemEnviou, 3, MPI_COMM_WORLD, &mpi_status);
                for(int lin= i * primeiro_chunk_linhas ; lin < (i+1) * primeiro_chunk_linhas ; lin++){
                    for(int col=0; col < M2_COLUMNS_LENGTH; col++){ //copia linha da matriz resultado_parcial para matriz_resultante
                        linhaAtualParcial = lin - (primeiro_chunk_linhas * i);
                        matriz_resultante[lin][col] = resultado_parcial[linhaAtualParcial][col];
                    }
                }
            }

        }else if(method == 2){
            if (M2_ROWS_LENGTH % ROWS_CHUNK_M2 > 0){
                printf("ROWS_CHUNCK_M2 deve ser multiplo de M2_ROWS_LENGTH\n");
                MPI_Finalize();
                return 0;
            }

            // envia chunk matriz A para slaves
            for(i = 0; i < comm_size - 1; i)
                MPI_Send(matriz_a[(primeiro_chunk_linhas) * i++], total_chunk_size_m1, MPI_INT, i, 1, MPI_COMM_WORLD);

            // distribui linhas da matriz B para slaves
            for(i = 0; i < M2_ROWS_LENGTH / ROWS_CHUNK_M2; i++){
                MPI_Bcast(matriz_b[ROWS_CHUNK_M2 * i], M2_COLUMNS_LENGTH * ROWS_CHUNK_M2, MPI_INT, 0, MPI_COMM_WORLD);
                MPI_Barrier(MPI_COMM_WORLD);
            }

            // recebe resultados dos slaves
            for(i = 0; i < comm_size - 1; i)
                MPI_Recv(matriz_resultante[(primeiro_chunk_linhas * i++)], total_chunk_size_mr, MPI_INT, i, 3, MPI_COMM_WORLD, &mpi_status);

        }else {
            aborta("Método inválido!");
        }

        printf("Matriz Resultante pelo método %d\n", method);
        mostraMatriz(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH, matriz_resultante);

        endtime = MPI_Wtime();
        printf("Tempo decorrido para o método %d: %f\n", method, endtime - starttime);
        //todo: free matrix
    }

    if (isSlave(comm_rank)){
        int (*resultado)[M2_COLUMNS_LENGTH];

        if (method == 1){
            int (*primeira_matriz)[M1_COLUMNS_LENGTH] = allocArray(M1_ROWS_LENGTH, primeiro_chunk_linhas);
            int (*segunda_matriz)[M2_COLUMNS_LENGTH] = allocArray(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH);

            MPI_Recv(primeira_matriz, total_chunk_size_m1, MPI_INT, 0, 1, MPI_COMM_WORLD, &mpi_status);
            MPI_Recv(segunda_matriz, M2_COLUMNS_LENGTH * M2_ROWS_LENGTH, MPI_INT, 0, 2, MPI_COMM_WORLD, &mpi_status);
            resultado = matrix_multiplier_sequential(primeiro_chunk_linhas, M1_COLUMNS_LENGTH, primeira_matriz, M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, segunda_matriz);
            MPI_Send(resultado, total_chunk_size_mr, MPI_INT, 0, 3, MPI_COMM_WORLD);
        }else if(method == 2){
            //todo: enviar pares de linhas ou chunks de linha da master pro slave ao invés de uma linha só
            int (*m1_slave)[M1_COLUMNS_LENGTH] = allocArray(primeiro_chunk_linhas, M1_COLUMNS_LENGTH);
            int (*m2_slave)[M2_COLUMNS_LENGTH] = allocArray(ROWS_CHUNK_M2, M2_COLUMNS_LENGTH);
            int (*resultado_slave)[M2_COLUMNS_LENGTH] = allocArray(primeiro_chunk_linhas, M2_COLUMNS_LENGTH);
            int j = 0, k = 0, l = 0;

            limpaMatriz(primeiro_chunk_linhas, M2_COLUMNS_LENGTH, resultado_slave);
            limpaMatriz(primeiro_chunk_linhas, M1_COLUMNS_LENGTH, m1_slave);

            MPI_Recv(m1_slave, total_chunk_size_m1, MPI_INT, 0, 1, MPI_COMM_WORLD, &mpi_status);

            for (k = 0; k < M2_ROWS_LENGTH / ROWS_CHUNK_M2; k++){
                MPI_Bcast(m2_slave, ROWS_CHUNK_M2 * M2_COLUMNS_LENGTH, MPI_INT, 0, MPI_COMM_WORLD);
                MPI_Barrier(MPI_COMM_WORLD);

                for(l = 0; l < ROWS_CHUNK_M2; l++){
                    for (i = 0; i < primeiro_chunk_linhas; i++){
                        for (j = 0; j < M2_COLUMNS_LENGTH; j++){
                            resultado_slave[i][j] += m1_slave[i][l + (k* ROWS_CHUNK_M2)] * m2_slave[l][j];
                        }
                    }
                }
            }

            MPI_Send(resultado_slave[0], total_chunk_size_mr, MPI_INT, 0, 3, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
