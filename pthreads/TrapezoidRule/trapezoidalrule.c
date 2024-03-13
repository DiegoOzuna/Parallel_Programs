//A quick note. This program is using pthreads as our parallel method.
//We are exploring the uses of busy-waiting, mutexes, and semaphores
//to enforce mutual exclusion in the critical section. We are also
//only using 4 threads.

//we are going to implement these in the trapezoidal rule. By default
//our function is going to be x^2, and our interval are [0,3]...

//IMPORTANT NOTE: You would run the code by doing ./trapezoidalrule <mode>
//                where mode is either mutex, busy, or semaphore.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

double a = 0.0, b = 3.0;   // interval [a, b]
int n = 1024;              // number of trapezoids
int thread_count = 4;      // number of threads

double h;                  // height of the trapezoids
double total;              // total integral
int flag;                  // flag for busy-waiting
pthread_mutex_t mutex;     // mutex for synchronization
sem_t *semaphore;          // semaphore for synchronization (note: semaphores work differently on macOS)


double f(double x) {       // function to integrate
    return x * x;          // can change this to other function
}

void* Trapezoidal_busy(void* rank) {
    long my_rank = (long) rank;
    double local_a = a + my_rank * h * n / thread_count;
    double local_b = local_a + n / thread_count * h;
    double x_i, local_total = (f(local_a) + f(local_b)) / 2.0;
    int i;

    for (i = 1; i <= n / thread_count - 1; i++) {
        x_i = local_a + i * h;
        local_total += f(x_i);
    }
    local_total *= h;

    while (flag != my_rank); //busy-waiting "lock"
    total += local_total;
    flag = (flag+1) % thread_count;

    return NULL;
}

void* Trapezoidal_mutex(void* rank){
    long my_rank = (long) rank;
    double local_a = a + my_rank * h * n / thread_count;
    double local_b = local_a + n / thread_count * h;
    double x_i, local_total = (f(local_a) + f(local_b)) / 2.0;
    int i;

    for (i = 1; i <= n / thread_count - 1; i++) {
        x_i = local_a + i * h;
        local_total += f(x_i);
    }
    local_total *= h;

    pthread_mutex_lock(&mutex);  //mutex lock
    total += local_total;
    pthread_mutex_unlock(&mutex);

    return NULL;

}

void* Trapezoidal_semaphore(void* rank){
    long my_rank = (long) rank;
    double local_a = a + my_rank * h * n / thread_count;
    double local_b = local_a + n / thread_count * h;
    double x_i, local_total = (f(local_a) + f(local_b)) / 2.0;
    int i;

    for (i = 1; i <= n / thread_count - 1; i++) {
        x_i = local_a + i * h;
        local_total += f(x_i);
    }
    local_total *= h;

    sem_wait(semaphore); //semaphore wait
    total += local_total;
    sem_post(semaphore); //semaphore signal

    return NULL;
}

int main(int argc, char* argv[]) {
    long thread;
    pthread_t* thread_handles;
    void* (*Trapezoidal)(void*);

    h = (b - a) / n;
    flag = 0;
    thread_handles = malloc(thread_count * sizeof(pthread_t));
    total = 0.0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <'busy' or 'mutex'>\n", argv[0]);
        exit(0);
    }


    //SELECT MODE OF TRAPEZOIDAL
    if (strcmp(argv[1], "busy") == 0) {
        Trapezoidal = Trapezoidal_busy;
    } else if (strcmp(argv[1], "mutex") == 0) {
        Trapezoidal = Trapezoidal_mutex;
        pthread_mutex_init(&mutex, NULL);
    } else if (strcmp(argv[1],"semaphore")== 0){
        Trapezoidal = Trapezoidal_semaphore;
        semaphore = sem_open("/semaphore", O_CREAT, 0644, 1);
        if (semaphore == SEM_FAILED) {
            perror("sem_open/semaphore");
            exit(EXIT_FAILURE);
        }
    }
    else {
        fprintf(stderr, "Invalid argument '%s'\n", argv[1]);
        exit(0);
    }


    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, Trapezoidal, (void*) thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    if (strcmp(argv[1], "mutex") == 0) {
        pthread_mutex_destroy(&mutex);
    }

    if(strcmp(argv[1], "semaphore") == 0){
        sem_close(semaphore);
        sem_unlink("/semaphore");
    }

    printf("With n = %d trapezoids, our estimate of the integral from %f to %f = %.15e\n", n, a, b, total);

    free(thread_handles);
    return 0;
}
