#include <stdio.h>
#include "mpi.h"


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

int main()
{
    MPI_Init (NULL, NULL);
    int comm_rank = 0;
    int comm_size = 0;

    MPI_Comm_rank (MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &comm_size);
  	MPI_Request isreq, irreq;
  	MPI_Status mpi_status;

    if(comm_size % 2 != 0) abort("Erro! número ímpar de tarefas.\n");

    if (isMaster(comm_rank)) printf("Comm size = %d\n", comm_size);

  	//if (isSlave(comm_rank))
  	    //MPI_Recv(&data_recebida, particao, MPI_FLOAT, 0, 15, MPI_COMM_WORLD, &mpi_status);

    MPI_Finalize ();
    return 0;
}
