#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mpi.h"

#define MATRIX_ONE_LINES_LENGTH 10
#define MATRIX_ONE_COLUMNS_LENGTH 10
#define MATRIX_TWO_LINES_LENGTH 10
#define MATRIX_TWO_COLUMNS_LENGTH 10

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
  int range = 100;
  int min = 0;
  srand(time(NULL));
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
  int range = 100;
  int min = 0;
  srand(time(NULL));
  int random_number;
  FILE *fileONE, *fileTWO;
  fileONE = fopen("fileone.bin", "wb+");
  fileTWO = fopen("filetwo.bin", "wb+");

  if(random){
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

void readMatrixFile(int array[MATRIX_ONE_LINES_LENGTH][MATRIX_ONE_COLUMNS_LENGTH]){
  printf("Reading matrix file\n");
  FILE *fileONE;
  fileONE = fopen("fileone.bin", "rb");
  fread(array, sizeof(int[MATRIX_ONE_LINES_LENGTH][MATRIX_ONE_COLUMNS_LENGTH]), 1, fileONE);
}

void mostraMatriz(int rows, int cols, int matrix[rows][cols]){
  int i, j;
  for(i = 0; i < MATRIX_ONE_LINES_LENGTH; i++){
    for(j = 0; j < MATRIX_ONE_COLUMNS_LENGTH; j++){
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
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
      readMatrixFile(firstMatrix);
      mostraMatriz(MATRIX_ONE_LINES_LENGTH, MATRIX_ONE_COLUMNS_LENGTH, firstMatrix);

      free(firstMatrix);
    }

  	//if (isSlave(comm_rank))
  	    //MPI_Recv(&data_recebida, particao, MPI_FLOAT, 0, 15, MPI_COMM_WORLD, &mpi_status);

    MPI_Finalize ();
    return 0;
}
