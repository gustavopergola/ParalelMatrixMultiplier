#include <stdio.h>
#include "mpi.h"

#define MATRIX_ONE_LINES_LENGTH 100
#define MATRIX_ONE_COLUMNS_LENGTH 100
#define MATRIX_TWO_LINES_LENGTH 100
#define MATRIX_TWO_COLUMNS_LENGTH 100

void abort(char[] error_msg){
    printf("%s", error_msg);
    MPI_Abort (MPI_COMM_WORLD, 1);
    return 1;
}

int isMaster(int rank){
    return rank == 0 ? 1 : NULL;
}

int isSlave(int rank){
  return isMaster(rank) == 1 ? NULL : 1;
}

void generateNewMatrixFile(){
  FILE *fileONE, *fileTWO;
  fileONE = FILE *fopen('fileone.bin', 'wb+');
  fileTWO = FILE *fopen('filetwo.bin', 'wb+');

  // int matrix_one_lines = MATRIX_ONE_LINES_LENGTH;
  // int matrix_one_cols = MATRIX_ONE_LINES_LENGTH;
  // int matrix_two_lines = MATRIX_TWO_LINES_LENGTH;
  // int matrix_two_cols = MATRIX_TWO_LINES_LENGTH;

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

    if(comm_size % 2 != 0) abort("Erro! número ímpar de tarefas.\n");

    if(matrizesNaoMultiplicaveis()) abort("Matrizes não são multiplicáveis!");

    if (isMaster(comm_rank)){
      printf("Comm size = %d\n", comm_size);
      if (argc == 2 && argv[1] == 'g') generateNewMatrixFile();
      else if(argc > 2) abort("Apenas um parametro esperado!");
    }

  	//if (isSlave(comm_rank))
  	    //MPI_Recv(&data_recebida, particao, MPI_FLOAT, 0, 15, MPI_COMM_WORLD, &mpi_status);

    MPI_Finalize ();
    return 0;
}
