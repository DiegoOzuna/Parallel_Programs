#include <stdio.h>
#include <mpi.h>

//NOTE: tree-structured global sum exists as a function in openMPI as MPI_Reduce(). We are just coding from "scratch" to show
//      understanding.


//Assume the values held within each process is equal to their ranks+1...
//this is done to showcase the parallel process only.

int main(int argc, char* argv[]){
    int rank, size, i;

    //Initialize MPI Environment//
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //////////////////////////////

    int local_sum = rank+1; //assume the local sum is rank+1
    int temp;

    printf("%d starting value: %d \n",rank, local_sum);

    for(int i =1; i < size; i*=2){
        if (rank % (2*i) == 0){       //if process is evenly divisible
            if(rank + i < size){      //and if the process has a neighbor to add to
                MPI_Recv(&temp, 1, MPI_INT, rank+i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //recieve the data from other process, and 
                printf("Process %d received %d from process %d\n", rank, temp, rank+i); //print out the communication
                local_sum += temp; //add into this process local sum
            }
        } else if ((rank - i) % (2*i) == 0){ //if process is odd/not evenly divisible, locate and send information towards the left (an even process)
            MPI_Send(&local_sum, 1, MPI_INT, rank-i, 0, MPI_COMM_WORLD);
            printf("Process %d sent %d to process %d\n", rank, local_sum, rank-i); //print out the communication
            break; //leave the loop (even side will handle the value passed, no other purpose for odd)
        }
    }

    if(rank ==0){ //rank 0 will hold the current global_sum
        printf("Global sum = %d\n", local_sum);
    }

    MPI_Finalize(); //close MPI environment
    return 0;
}
