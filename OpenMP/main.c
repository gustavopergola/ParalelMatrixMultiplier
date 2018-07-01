#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

#define M1_ROWS_LENGTH 1800
#define M1_COLUMNS_LENGTH 1800
#define M2_ROWS_LENGTH 1800
#define M2_COLUMNS_LENGTH 3600

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
    return malloc(sizeof(int[rows][cols]) ); // allocate 1 2D-array
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
    double begin = omp_get_wtime();
    int (*resultMatrix)[M2_COLUMNS_LENGTH] = matrix_multiplier_sequential(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, matriz_a, M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, matriz_b);

    double end = omp_get_wtime();
    double time_spent = end - begin;
    printf("Tempo decorrido utilizando o método sequencial: %f\n", time_spent);

    //printf("Matriz resultante (SEQUENCIAL):\n");
    //mostraMatriz(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH, resultMatrix);
    free(resultMatrix);
}

void* matrix_multiplier_openmp(int rowsA, int colsA, int matrixA[rowsA][colsA], int rowsB, int colsB, int matrixB[rowsB][colsB]){

    int i,j,k;
    int (*result)[colsB] = allocArray(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH);
    #pragma omp parallel shared(matrixA,matrixB,result) private(i,j,k)
    {
        #pragma omp for  schedule(static)
        for (i=0; i<rowsA; i++){
            for (j=0; j<colsB; j++){
                result[i][j] = 0;
                for (k=0; k<rowsB; k++){
                    result[i][j] = result[i][j] + matrixA[i][k]*matrixB[k][j];
                }
            }
        }
    }
    return result;
}

void calcula_matriz_openmp(int (*matriz_a)[M1_COLUMNS_LENGTH], int (*matriz_b)[M2_COLUMNS_LENGTH]){
    double begin = omp_get_wtime();
    int (*resultMatrix)[M2_COLUMNS_LENGTH] = matrix_multiplier_openmp(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, matriz_a, M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, matriz_b);
    double end = omp_get_wtime();
    double time_spent = end - begin;
    printf("Tempo decorrido utilizando OpenMP: %f\n", time_spent);
    //mostraMatriz(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH, resultMatrix);
    free(resultMatrix);
}


int main(int argc, char *argv[])
{
    int i;
    for(i=1; i < argc; i++){
        if (strcmp(argv[i], "g") == 0) generateNewMatrixFile(0);
        else if (strcmp(argv[i], "g+") == 0) generateNewMatrixFile(1);
    }

    if(matrizesNaoMultiplicaveis()) return 0;
    int (*matriz_a)[M1_COLUMNS_LENGTH] = allocArray(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH);
    int (*matriz_b)[M2_COLUMNS_LENGTH] = allocArray(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH);
    int (*matriz_resultante)[M2_COLUMNS_LENGTH] = allocArray(M1_ROWS_LENGTH, M2_COLUMNS_LENGTH);


    readMatrixFiles(matriz_a, matriz_b);

    //printf("Matriz A:\n");
    //mostraMatriz(M1_ROWS_LENGTH, M1_COLUMNS_LENGTH, matriz_a);
    //printf("Matriz B:\n");
    //mostraMatriz(M2_ROWS_LENGTH, M2_COLUMNS_LENGTH, matriz_b);

    calcula_matriz_resultante_sequencial(matriz_a, matriz_b);
    printf("calculo de matriz sequencial terminada!\n");

    calcula_matriz_openmp(matriz_a, matriz_b);
    printf("calculo de matriz utilizando OpenMP terminada!\n");
}
