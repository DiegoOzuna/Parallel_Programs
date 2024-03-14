//this program is to analyze the thread start up and dismantle time. We want to see if there is
//any difference in how many threads are being made. a list has been made and results will be printed out.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Define different thread counts
const int thread_counts[] = {2, 4, 6, 8, 10, 20};

void* thread_work(void* arg) {
    // No work is done in the thread. We only care to measure
    // thread creation time and deletion
    return NULL;
}

int main() {

    for (int i = 0; i < sizeof(thread_counts) / sizeof(thread_counts[0]); ++i) {
        int num_threads = thread_counts[i];
        pthread_t threads[num_threads];

        struct timeval start_time, end_time;
        double total_time;
        gettimeofday(&start_time, NULL);

        // Create threads
        for (int j = 0; j < num_threads; ++j) {
            pthread_create(&threads[j], NULL, thread_work, NULL);
        }

        // Wait for threads to finish
        for (int j = 0; j < num_threads; ++j) {
            pthread_join(threads[j], NULL);
        }

        gettimeofday(&end_time, NULL);

        // Calculate total time
        total_time = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_usec - start_time.tv_usec) / 1e6;

        // Calculate average time per thread
        double avg_time = total_time / num_threads;

        printf("Number of threads: %d\n", num_threads);
        printf("Total time taken: %.6f seconds\n", total_time);
        printf("Average time per thread: %.6f seconds\n\n", avg_time);
    }

    return 0;
}


