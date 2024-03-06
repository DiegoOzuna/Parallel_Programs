#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 1000 // number of data points
#define M 10 // number of bins

int main(int argc, char** argv) {
    int rank, size, i;
    int data[N], recvbuf[N], histogram[M] = {0}, recvhist[M] = {0};

    MPI_Init(&argc, &argv); //initialize mpi process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //returns the unique identifier for a process in a group
    MPI_Comm_size(MPI_COMM_WORLD, &size); //returns the total num of processes in group

    if (rank == 0) { //we first have to generate numbers for our histogram...
        // Process 0 reads the input data
        for (i = 0; i < N; i++) {
            data[i] = rand() % M; // generate random numbers between 0 and M-1
        }
    }

    // Distribute the data among the processes
    MPI_Scatter(data, N/size, MPI_INT, recvbuf, N/size, MPI_INT, 0, MPI_COMM_WORLD);
    //params are : data, num of elements to send to each process, specify data type of each element, buffer that holds data which is recieved on each process, 
    //             num of elements in recieve buffer, data type of each element in buffer, rank of the sending process within the specified communicator, the MPI_Comm communicator handle

    // Each process computes its local histogram
    for (i = 0; i < N/size; i++) {
        histogram[recvbuf[i]]++;
    }

    // Print out the local histogram of each process
    printf("Process %d local histogram:\n", rank);
    for (i = 0; i < M; i++) {
        printf("The frequency of %d: was %d\n", i, histogram[i]);
    }

    // Gather the local histograms into the global histogram
    MPI_Reduce(histogram, recvhist, M, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    //params are : data, recieve buffer, specify the number of elements in the buffer, specify data type of each element,
    //             the operation that is going to be applied to the data, rank of the sending process within the specified communicator, the MPI_Comm communicator handle

    if (rank == 0) {
        // Process 0 prints out the histogram
        printf("Histogram:\n");
        for (i = 0; i < M; i++) {
            printf("The frequency of %d: was %d\n", i, recvhist[i]);
        }
    }

    MPI_Finalize(); //close opened resources from MPI
    return 0;
}
