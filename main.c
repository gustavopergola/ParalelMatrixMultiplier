#include <stdio.h>
#include <stdbool.h>
#include "mpi.h"

#define MATRIX_ONE_LINES_LENGTH 100
#define MATRIX_ONE_COLUMNS_LENGTH 100
#define MATRIX_TWO_LINES_LENGTH 100
#define MATRIX_TWO_COLUMNS_LENGTH 100

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

void generateNewMatrixFile(){
  printf("Generating new matrix file\n");
  FILE *fileONE, *fileTWO;
  fileONE = fopen("fileone.bin", "wb+");
  fileTWO = fopen("filetwo.bin", "wb+");

  int i, j;
  for(i = 0; i < MATRIX_ONE_LINES_LENGTH; i++){
    for(j = 0; j < MATRIX_ONE_COLUMNS_LENGTH; j++){
      fwrite(&j, sizeof(int), 1,fileONE);
    }
  }

  for(i = 0; i < MATRIX_TWO_LINES_LENGTH; i++){
    for(j = 0; j < MATRIX_TWO_COLUMNS_LENGTH; j++){
      fwrite(&j, sizeof(int), 1, fileTWO);
    }
  }

  fclose(fileONE);
  fclose(fileTWO);
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
      for(i=1; i < argc; i++)
       if (*argv[i] == 'g') generateNewMatrixFile();
    }

    

  	//if (isSlave(comm_rank))
  	    //MPI_Recv(&data_recebida, particao, MPI_FLOAT, 0, 15, MPI_COMM_WORLD, &mpi_status);

    MPI_Finalize ();
    return 0;
}
