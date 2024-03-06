#include <stdio.h>
#include <mpi.h>


//NOTE: butterfly global sum exists as a function in openMPI as MPI_Allreduce(). We are just coding from scratch to show
//      understanding.

//NOTE: Butterfly globalsum will do the same process to find the globalsum, key difference is it will
//      propogate that information back to the other processes too.

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

    // Find the next power of 2     < ---- This will not change the behavior of when -np is even, but is used for when -np is odd
    int size_p2 = 1;
    while(size_p2 < size) size_p2 *= 2;

    // Propagate the global sum back to all processes
    //NOTE: This loop is esentially the reverse of the first loop. This is what would allow us to "mirror" the tree to gain butterfly pattern
    for(int i = size_p2/2; i >= 1; i /= 2){ //idea here is that we have processes pass eachother the result, which would halve the process
        if(rank < size && rank % (2*i) == 0){ //if we are still within bounds and are evenly divisible
            if(rank + i < size){ //making sure "neighbor" exists
                MPI_Send(&local_sum, 1, MPI_INT, rank+i, 0, MPI_COMM_WORLD); //update neighbor with information
                printf("Process %d informed %d\n", rank, rank+i); //print out the communication
            }
        } else if(rank < size && (rank - i) % (2*i) == 0){ //basically stating "If I am on right hand side"
            MPI_Recv(&temp, 1, MPI_INT, rank-i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //"grab info from left"
            printf("Process %d updated by process %d \n", rank, rank-i); //print out the communication
            local_sum = temp; //"update self"
        }
    }


    printf("Process %d received the global sum = %d\n", rank, local_sum); //show that each proccess has the global sum now

    MPI_Finalize(); //close MPI environment
    return 0;
}
