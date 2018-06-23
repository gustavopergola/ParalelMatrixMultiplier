#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mpi.h"

#define M1_ROWS_LENGTH 4
#define M1_COLUMNS_LENGTH 4
#define M2_ROWS_LENGTH 4
#define M2_COLUMNS_LENGTH 4

int aborta(char *error_msg){
    printf("%s", error_msg);
    MPI_Abort (MPI_COMM_WORLD, 1);
    return 1;
}

bool isMaster(int rank){
    return rank == 0 ? true : false;
}

bool isSlave(int rank){
    return !isMaster(rank);
}

bool matrizesNaoMultiplicaveis(){
    return M1_COLUMNS_LENGTH != M2_ROWS_LENGTH;
}

void writeMatrix(int rows, int cols, FILE *file){
    int i, j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            fwrite(&j, sizeof(int), 1,file);
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

void calcula_matriz_transposta (int linhas, int colunas, int matrixA[linhas][colunas], int matrixAT[colunas][linhas]) {
    int i, j;
    for (i = 0; i < linhas; i++)
        for (j = 0; j < colunas; j++)
            matrixAT[j][i] = matrixA[i][j];
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

void* matrix_multiplier_sequential(int rowsA, int colsA, int matrixA[rowsA][colsA], int rowsB, int colsB, int matrixB[rowsB][colsB]){
    int (*result)[colsB] = malloc(sizeof(int[colsA][rowsB]));

    int i,j,k;
    for (i=0; i<rowsA; i++){
        for (j=0; j<colsB; j++){
            result[i][j] = 0;
            for (k=0; k<colsA; k++){
                result[i][j] = result[i][j] + matrixA[i][k]*matrixB[k][j];
            }
        }
    }
    return result;

}

void calcula_matriz_resultante_sequencial(int (*firstMatrix)[M1_COLUMNS_LENGTH], int (*secondMatrix)[M2_COLUMNS_LENGTH]){
    double starttime = 0, endtime = 0;
    starttime = MPI_Wtime();
    int (*resultMatrix)[M2_COLUMNS_LENGTH] = matrix_multiplier_sequential(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, firstMatrix, M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, secondMatrix);
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
    int segundo_chunk_linhas;

    if (comm_size > 1){
         primeiro_chunk_linhas = M1_ROWS_LENGTH   / (comm_size - 1);
         segundo_chunk_linhas = M2_COLUMNS_LENGTH / (comm_size - 1);
    }

    if (isMaster(comm_rank)){
        printf("Comm size = %d\n", comm_size);
        int i;
        for(i=1; i < argc; i++){
            if (strcmp(argv[i], "g") == 0) generateNewMatrixFile(0);
            else if (strcmp(argv[i], "g+") == 0) generateNewMatrixFile(1);
            else if (strcmp(argv[i], "s") == 0) sequencial = 1;//random matrix
            else if (strcmp(argv[i], "m1") == 0){ printf("Executando método 1\n"); method = 1; }
            else if (strcmp(argv[i], "m2") == 0){ printf("Executando método 2\n"); method = 2; }
        }

        if(matrizesNaoMultiplicaveis()) return aborta("Matrizes não são multiplicáveis!\n");
        int (*firstMatrix)[M1_COLUMNS_LENGTH] = allocArray(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH);
        int (*secondMatrix)[M2_COLUMNS_LENGTH] = allocArray(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH);
        int (*segunda_matriz_transposta)[M2_ROWS_LENGTH] = allocArray(M2_COLUMNS_LENGTH, M2_ROWS_LENGTH);
        int (*matriz_resultante)[M2_COLUMNS_LENGTH] = malloc(sizeof(int[M1_ROWS_LENGTH][M2_COLUMNS_LENGTH]));

        readMatrixFiles(firstMatrix, secondMatrix);

        calcula_matriz_transposta(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, secondMatrix, segunda_matriz_transposta);

        printf("Matriz A:\n");
        mostraMatriz(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, firstMatrix);
        printf("Matriz B:\n");
        mostraMatriz(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, secondMatrix);
        //printf("Matriz B transposta:\n");
        //mostraMatriz(M2_COLUMNS_LENGTH, M2_ROWS_LENGTH, segunda_matriz_transposta);

        if (sequencial == 1){
            calcula_matriz_resultante_sequencial(firstMatrix, secondMatrix);
            printf("calculo de matriz sequencial terminada!\n");
            MPI_Finalize ();
            return 0;
        }

        if(comm_size == 1){
            printf("Não é possível utilizar os métodos paralelos com apenas uma thread\ntente utilizar 2 threads.\n");
            MPI_Finalize ();
            return 0;
        }

        if(method == 1 || method == 0){
            double starttime = 0, endtime = 0;
            starttime = MPI_Wtime();


            // distribui pedacos iguais da matriz para os pocessos
            // TODO: matriz não perfeitamente divisível

            int destination;
            for(i = 0; i < comm_size - 1; i++){
                // considerando 1 linha por thread inicialmente para simplicidade
                // todo: enviar pedaços da matriz
                destination = i + 1;

                MPI_Send(firstMatrix[(primeiro_chunk_linhas) * i], primeiro_chunk_linhas * M1_COLUMNS_LENGTH, MPI_INT, destination, 1, MPI_COMM_WORLD);
            }

            for(i = 0; i < comm_size - 1; i++){
                // considerando 1 linha por thread inicialmente para simplicidade
                // todo: enviar pedaços da matriz
                destination = i + 1;
                MPI_Send(secondMatrix, M2_COLUMNS_LENGTH * M2_ROWS_LENGTH, MPI_INT, destination, 2, MPI_COMM_WORLD);
            }

            int (*resultado_parcial)[M2_COLUMNS_LENGTH] = allocArray(primeiro_chunk_linhas, M2_COLUMNS_LENGTH);

            int quemEnviou, linhaAtualParcial;

            for(i = 0; i < comm_size -1; i++){
                quemEnviou = i + 1;
                MPI_Recv(resultado_parcial, M1_ROWS_LENGTH * M2_COLUMNS_LENGTH, MPI_INT, quemEnviou, 3, MPI_COMM_WORLD, &mpi_status);
                for(int lin= i * primeiro_chunk_linhas ; lin < (i+1)*primeiro_chunk_linhas ; lin++){
                    for(int col=0; col < M2_COLUMNS_LENGTH; col++){ //copia linha da matriz resultado_parcial para matriz_resultante
                        linhaAtualParcial = lin - (primeiro_chunk_linhas * i);
                        matriz_resultante[lin][col] = resultado_parcial[linhaAtualParcial][col];
                    }
                }

            }

            printf("Matriz Resultante pelo método 1\n");
            mostraMatriz(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH, matriz_resultante);

            endtime = MPI_Wtime();
            printf("Tempo decorrido para o método 1: %f\n", endtime-starttime);

        }else if(method == 2){
            // todo: segundo metodo paralelo
        }

        //todo: free matrix
    }

    if (isSlave(comm_rank)){
        int (*primeira_matriz)[M1_COLUMNS_LENGTH] = allocArray(primeiro_chunk_linhas, M1_COLUMNS_LENGTH);
        int (*segunda_matriz)[M2_ROWS_LENGTH] = allocArray(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH);

        MPI_Recv(primeira_matriz, primeiro_chunk_linhas * M1_COLUMNS_LENGTH, MPI_INT, 0, 1, MPI_COMM_WORLD, &mpi_status);
        MPI_Recv(segunda_matriz,  M2_COLUMNS_LENGTH * M2_ROWS_LENGTH   , MPI_INT, 0, 2, MPI_COMM_WORLD, &mpi_status);

        //printf("Matriz A slave%d\n", comm_rank);
        //mostraMatriz(primeiro_chunk_linhas, M1_COLUMNS_LENGTH, primeira_matriz);

        //printf("Matriz B slave %d\n", comm_rank);
        //mostraMatriz(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, segunda_matriz);

        int (*resultado)[M2_COLUMNS_LENGTH] = matrix_multiplier_sequential(primeiro_chunk_linhas, M1_COLUMNS_LENGTH, primeira_matriz, M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, segunda_matriz);

        //printf("Resultado slave %d\n", comm_rank);
        //mostraMatriz(primeiro_chunk_linhas, M2_COLUMNS_LENGTH, resultado);

        MPI_Send(resultado, primeiro_chunk_linhas * M2_COLUMNS_LENGTH, MPI_INT, 0, 3, MPI_COMM_WORLD);
    }

    MPI_Finalize ();
    return 0;
}
