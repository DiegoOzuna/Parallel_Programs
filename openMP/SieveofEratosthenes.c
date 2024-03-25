#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char* argv[]) {
    int n = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    int i, j;

    // Calculate the square root of n
    // is used as an optimization because we do not need to look through whole list, as proof was shown that
    // the prime number would remove at most sqrt n nums from list.
    int sqrt_n = sqrt(n);

    // Allocate memory for the array
    int* prime = (int*) malloc(sizeof(int) * n);
    for(i = 2; i < n; i++) {
        prime[i] = 1;
    }

    omp_set_num_threads(num_threads); //this will set the thread amount to be used from this point on...

    // Start the timer ; we only care about the processing time to finish finding the primes, not really the initialization of the threads
    double start_time = omp_get_wtime();

    #pragma omp parallel for private(j)
    for(i = 2; i <= sqrt_n; i++) {
        // If prime[i] is not changed, then it is a prime
        if (prime[i]) {
            // Update all multiples of i
            for(j = i * i; j <= n; j += i)
                prime[j] = 0;
        }
    }

    // Stop the timer
    double end_time = omp_get_wtime();
    double time_elapsed = end_time - start_time;

    // // Print all prime numbers
    // for(i = 2; i <= n; i++) {
    //     if(prime[i])
    //         printf("%d ", i);
    // }

    printf("\nTime elapsed: %f seconds\n", time_elapsed);

    free(prime);

    return 0;
}
