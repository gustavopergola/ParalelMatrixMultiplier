#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mpi.h"

#define MATRIX_ONE_LINES_LENGTH 4
#define MATRIX_ONE_COLUMNS_LENGTH 4
#define MATRIX_TWO_LINES_LENGTH 4
#define MATRIX_TWO_COLUMNS_LENGTH 4

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
  return MATRIX_ONE_LINES_LENGTH != MATRIX_TWO_COLUMNS_LENGTH;
}

void writeMatrix(FILE *file){
  int i, j;
  for(i = 0; i < MATRIX_ONE_LINES_LENGTH; i++){
    for(j = 0; j < MATRIX_ONE_COLUMNS_LENGTH; j++){
      fwrite(&j, sizeof(int), 1,file);
    }
  }
}

void writeRandomMatrix(FILE *file){
  int range = 10;
  int min = 0;
  int random_number;
  int i, j;
  for(i = 0; i < MATRIX_ONE_LINES_LENGTH; i++){
    for(j = 0; j < MATRIX_ONE_COLUMNS_LENGTH; j++){
      random_number = rand() % range + min;
      fwrite(&random_number, sizeof(int), 1, file);
    }
  }
}

void generateNewMatrixFile(int random){
  //1: random matrix  0: not random
  printf("Generating new matrix file\n");
  FILE *fileONE, *fileTWO;
  fileONE = fopen("fileone.bin", "wb+");
  fileTWO = fopen("filetwo.bin", "wb+");

  if(random){
    srand(time(NULL));
    writeRandomMatrix(fileONE);
    writeRandomMatrix(fileTWO);
  }else{
    writeMatrix(fileONE);
    writeMatrix(fileTWO);
  }

  fclose(fileONE);
  fclose(fileTWO);
}

void* allocArray (int rows, int cols)
{
  return malloc( sizeof(int[rows][cols]) ); // allocate 1 2D-array
}

void readMatrixFiles(int array1[MATRIX_ONE_LINES_LENGTH][MATRIX_ONE_COLUMNS_LENGTH], 
                     int array2[MATRIX_TWO_LINES_LENGTH][MATRIX_TWO_COLUMNS_LENGTH]){
  printf("Reading matrix files\n");
  FILE *fileONE, *fileTWO;
  fileONE = fopen("fileone.bin", "rb");
  fileTWO = fopen("filetwo.bin", "rb");
  fread(array1, sizeof(int[MATRIX_ONE_LINES_LENGTH][MATRIX_ONE_COLUMNS_LENGTH]), 1, fileONE);
  fread(array2, sizeof(int[MATRIX_TWO_LINES_LENGTH][MATRIX_TWO_COLUMNS_LENGTH]), 1, fileTWO);
}

void mostraMatriz(int rows, int cols, int matrix[rows][cols]){
  int i, j;
  for(i = 0; i < MATRIX_ONE_LINES_LENGTH; i++){
    for(j = 0; j < MATRIX_ONE_COLUMNS_LENGTH; j++){
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void* matrix_multiplier_sequential(int rowsA, int colsA, int matrixA[rowsA][colsA], int rowsB, int colsB, int matrixB[rowsB][colsB]){
    int (*result)[MATRIX_ONE_COLUMNS_LENGTH] = malloc(sizeof(int[colsA][rowsB]));
    
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

int main(int argc, char *argv[])
{
    MPI_Init (NULL, NULL);
    int comm_rank = 0;
    int comm_size = 0;

    MPI_Comm_rank (MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &comm_size);
  	MPI_Request isreq, irreq;
  	MPI_Status mpi_status;

    if(comm_size % 2 != 0){
      printf("%d\n", comm_size);
      return aborta("Erro! número ímpar de tarefas.\n");
    }   

    if(matrizesNaoMultiplicaveis()) return aborta("Matrizes não são multiplicáveis!\n");

    if (isMaster(comm_rank)){
      printf("Comm size = %d\n", comm_size);
      int i;
      for(i=1; i < argc; i++){
        if (strcmp(argv[i], "g") == 0) generateNewMatrixFile(0);
        else if (strcmp(argv[i], "g+") == 0) generateNewMatrixFile(1);//random matrix
      }
      int (*firstMatrix)[MATRIX_ONE_COLUMNS_LENGTH] = allocArray(MATRIX_ONE_LINES_LENGTH, MATRIX_ONE_COLUMNS_LENGTH);
      int (*secondMatrix)[MATRIX_TWO_COLUMNS_LENGTH] = allocArray(MATRIX_TWO_LINES_LENGTH, MATRIX_TWO_COLUMNS_LENGTH);

      readMatrixFiles(firstMatrix, secondMatrix);
      printf("Matriz A:\n");
      mostraMatriz(MATRIX_ONE_LINES_LENGTH, MATRIX_ONE_COLUMNS_LENGTH, firstMatrix);
      printf("Matriz B:\n");
      mostraMatriz(MATRIX_TWO_LINES_LENGTH, MATRIX_TWO_COLUMNS_LENGTH, secondMatrix);

      int (*resultMatrix)[MATRIX_ONE_COLUMNS_LENGTH] = matrix_multiplier_sequential(MATRIX_ONE_LINES_LENGTH, MATRIX_ONE_COLUMNS_LENGTH, firstMatrix, MATRIX_TWO_LINES_LENGTH, MATRIX_TWO_COLUMNS_LENGTH, secondMatrix);
      printf("Matriz resultante:\n");
      mostraMatriz(MATRIX_ONE_COLUMNS_LENGTH, MATRIX_TWO_LINES_LENGTH, resultMatrix);
      free(firstMatrix);
    }
  	//if (isSlave(comm_rank))
  	    //MPI_Recv(&data_recebida, particao, MPI_FLOAT, 0, 15, MPI_COMM_WORLD, &mpi_status);

    MPI_Finalize ();
    return 0;
}
