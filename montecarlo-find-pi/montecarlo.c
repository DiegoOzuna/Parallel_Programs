#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

long double random_long_double() {
    return (long double)rand() / (long double)RAND_MAX * 2.0L - 1.0L;
}

int main(int argc, char** argv) {
    int rank, size;
    int toss =0;
    long double x, y, distance_squared=0;
    long long int total_toss, toss_per_p, number_in_circle=0, total_num_in_circle =0;
    double pi_estimate =0;
    srand((unsigned int)time(NULL));

    MPI_Init(&argc, &argv); //initialize mpi process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //returns the unique identifier for a process in a group
    MPI_Comm_size(MPI_COMM_WORLD, &size); //returns the total num of processes in group


    //Ask for n ...
    if (rank == 0) {
        printf("Input total number of tosses...: \n"); 
        scanf("%lld", &total_toss);
    }
    toss_per_p = total_toss / size; //make sure to distribute the tosses equally to each processor
    MPI_Bcast(&toss_per_p, 1, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD); //let every process know what total_toss was set to....


    printf("%d has recieved %lld total toss \n",rank, toss_per_p);
    for(toss = 1; toss < toss_per_p; toss++){             //Each process will calculate their own distances from the tosses
        x = random_long_double();
        y = random_long_double();
        distance_squared = x*x + y*y;
        if(distance_squared <= 1){
            number_in_circle++;
        }
    }
    printf("%d just got out of for loop distance \n", rank);
    printf("%d has %lld in the circle \n", rank, number_in_circle);

    if(rank == 0){
        MPI_Reduce(&number_in_circle, &total_num_in_circle, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    } else {
        MPI_Reduce(&number_in_circle, NULL, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    }


    if(rank==0){
        pi_estimate = 4*total_num_in_circle/((double) total_toss);
        printf("The estimate of pi: %f \nfrom %lld total tosses", pi_estimate, total_num_in_circle);
    }

    MPI_Finalize();
    return 0;
}