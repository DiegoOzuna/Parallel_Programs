/*
Implement a producer-consumer program in which some of the
threads are producers and others are consumers. The producers read
text from a collection of files, one per producer. They insert 
lines of text into a single shared queue. The consumers take the 
lines of text and tokenize them. Tokens are "words" sepearted by white
space. When a consumer finds a token, it writes it to stdout.
*/


#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define QUEUE_SIZE 10
#define MAX_LINE_LENGTH 1000

char *queue[QUEUE_SIZE];
int readIndex = 0;
int writeIndex = 0;

omp_lock_t lock;
int production_done = 0;

void produce(char *filenames[], int num_files) {
    for (int i = 0; i < num_files; i++) {
        char *filename = filenames[i];
        FILE *file = fopen(filename, "r");
        char *line = malloc(MAX_LINE_LENGTH * sizeof(char));

        while (fgets(line, MAX_LINE_LENGTH, file)) {
            omp_set_lock(&lock);
            while ((writeIndex + 1) % QUEUE_SIZE == readIndex) {
                omp_unset_lock(&lock);  // release lock if queue is full
                omp_set_lock(&lock);    // try to acquire lock again
            }

            queue[writeIndex] = strdup(line);
            writeIndex = (writeIndex + 1) % QUEUE_SIZE;

            printf("Producer: inserted line from %s into queue at position %d\n", filename, writeIndex);
            omp_unset_lock(&lock);
        }

        free(line);
        fclose(file);
    }

    // Insert sentinel value into the queue to signal the end of production
    omp_set_lock(&lock);
    while ((writeIndex + 1) % QUEUE_SIZE == readIndex) {
        omp_unset_lock(&lock);  // release lock if queue is full
        omp_set_lock(&lock);    // try to acquire lock again
    }

    queue[writeIndex] = NULL;
    writeIndex = (writeIndex + 1) % QUEUE_SIZE;
    production_done = 1;

    omp_unset_lock(&lock);
}

void consume() {
    while (1) {
        omp_set_lock(&lock);
        while (readIndex == writeIndex && !production_done) {
            omp_unset_lock(&lock);  // release lock if queue is empty
            omp_set_lock(&lock);    // try to acquire lock again
        }

        if (readIndex == writeIndex && production_done) {
            omp_unset_lock(&lock);
            break;  // no more lines to read
        }

        char *line = queue[readIndex];
        readIndex = (readIndex + 1) % QUEUE_SIZE;

        printf("Consumer: reading line from queue at position %d\n", readIndex);
        omp_unset_lock(&lock);

        char *token = strtok(line, " ");
        while (token) {
            printf("Consumer: found token '%s'\n", token);
            token = strtok(NULL, " ");
        }

        free(line);
    }
}

int main() {
    char *filenames[] = {"file1.txt", "file2.txt", "file3.txt", "file4.txt"};

    omp_init_lock(&lock);

    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int tid = omp_get_thread_num();

        if (tid < NUM_THREADS / 2) {
            // Each producer thread handles half of the files
            produce(&filenames[tid * (sizeof(filenames) / sizeof(char*) / (NUM_THREADS / 2))], sizeof(filenames) / sizeof(char*) / (NUM_THREADS / 2));
        } else {
            consume();
        }
    }

    omp_destroy_lock(&lock);

    return 0;
}










