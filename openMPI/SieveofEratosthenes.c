#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char* argv[]){
    //make sure that a command-line is passed to set up
    if(argc < 2) {
        printf("Please provide a command-line argument.\n");
        return 1;
    }

    //set search_max to the command-line passed
    int search_max = strtol(argv[1], NULL, 10);
    
    //grab the sqrt of search
    int sqrt_search_max = (int)sqrt(search_max);

    //track the time...
    clock_t begintime = clock();

    //allocate memory to hold for primes
    int* primes = (int*)malloc((search_max+1) * sizeof(int));
    
    //we will use the index as our number count and state, 1 means prime, 0 means not prime
    //we start by initializing everything as prime, and will put in 0s in our sifting process
    for(int i = 0; i <= search_max; i++){
        primes[i] = i;
    }

    //set 1 and 0 to not prime (logic will skip over these)
    primes[0] = 0;
    primes[1] = 0;

    //initialize the openMPI parallel process
    MPI_Init(&argc, &argv);

    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    //split the data 
    int chunk_size = search_max / size;
    int start = rank * chunk_size + 1;
    int end = (rank == size - 1) ? search_max : (rank + 1) * chunk_size;
    
    // Print the portion of the array received by each process
    printf("Process %d received numbers from %d to %d: ", rank, start, end);
    for(int i = start; i <= end; i++){
        printf("%d ", primes[i]);
    }
    printf("\n");

    //begin sift from 2nd 
    for(int i = 2; i <= sqrt_search_max; i++) {
        int current_prime = i;

        for(int j = 2; j <= end; j++){
            if(primes[j] != 0 && primes[j] % current_prime == 0 && primes[j] != current_prime){
                primes[j] = 0;
            }
        }

        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, primes, chunk_size, MPI_INT, MPI_COMM_WORLD);
    }

    printf("Prime numbers found by process %d: ", rank);
    for(int i = start; i <= end; i++){
        if(primes[i] != 0){
            printf("%d ", primes[i]);
        }
    }
    printf("\n");

    free(primes);

    printf("End of program reached.\n");

    MPI_Finalize();

    clock_t endtime = clock();
    double time_spent = (double)(endtime - begintime) / CLOCKS_PER_SEC;
    printf("The program took %f seconds to run. \n", time_spent);

    return 0;
}