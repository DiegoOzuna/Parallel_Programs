#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char* argv[]){
    //Take in a thread count as a command line arguement
    int thread_count = strtol(argv[1],NULL, 10);

    //Take in a n value as a command line argument
    int n = strtol(argv[2], NULL, 10);

    //calculate the number pi using a summation
    double factor = 1.0;
    double sum = 0.0;

    #pragma omp parallel for num_threads(thread_count) reduction(+: sum) private(factor)
    for(int k = 0; k < n; k++){
        if (k%2 == 0) factor = 1.0;
        else factor = -1.0;

        sum += factor/ (2.0 * k + 1);
    }

    double pi_approx = 4.0 * sum;

    printf("Pi approximate based off %d terms is:\n",n);
    printf("%.10f\n", pi_approx);

    return 0;
}