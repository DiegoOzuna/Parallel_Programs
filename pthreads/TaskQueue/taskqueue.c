#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define TASK_QUEUE_SIZE 20

typedef struct task { //for linked list
    int data;
    struct task *next;
} Task;

typedef struct task_queue { //for queue
    Task *head;
    Task *tail;
    int size;
} TaskQueue;

//to ensure sync
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_condition = PTHREAD_COND_INITIALIZER;
int done = 0; //flag for threads to know if tasks are done

TaskQueue task_queue;

//for simplicity task queue will have two functions. enqueue and dequeue.
void enqueue(TaskQueue *queue, Task *task) {
    pthread_mutex_lock(&queue_mutex);
    
    if (queue->size == TASK_QUEUE_SIZE) {
        fprintf(stderr, "Task queue is full. Task not added.\n");
        pthread_mutex_unlock(&queue_mutex);
        return;
    }
    
    if (queue->size == 0) {
        queue->head = task;
        queue->tail = task;
    } else {
        queue->tail->next = task;
        queue->tail = task;
    }
    queue->size++;
    
    pthread_cond_signal(&queue_condition);
    
    pthread_mutex_unlock(&queue_mutex);
}


Task *dequeue(TaskQueue *queue) {
    pthread_mutex_lock(&queue_mutex);
    
    while (queue->size == 0 && !done) {
        pthread_cond_wait(&queue_condition, &queue_mutex);
    }
    
    if (queue->size == 0) {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
    }
    
    Task *task = queue->head;
    queue->head = task->next;
    queue->size--;
    
    pthread_mutex_unlock(&queue_mutex);
    
    return task;
}

//for simplicity sake, threads are just dequeueing
void *task_handler(void *arg) {
    while (1) {
        Task *task = dequeue(&task_queue);
        
        if (task == NULL) {
            break;
        }
        
        // Perform linked list operation (for demonstration purpose)
        printf("Task %d processed by thread %p\n", task->data, (void *)pthread_self());
        
        free(task);
    }
    
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    task_queue.head = NULL;
    task_queue.tail = NULL;
    task_queue.size = 0;
    
    //initialize threads, with task handler function
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, task_handler, NULL);
    }
    
    // Generate tasks
    for (int i = 0; i < TASK_QUEUE_SIZE; i++) {
        Task *task = (Task*)malloc(sizeof(Task));
        task->data = i;
        task->next = NULL;
        
        enqueue(&task_queue, task);
    }
    
    // Signal no more tasks
    done = 1;
    pthread_cond_broadcast(&queue_condition);
    
    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    //clean up
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_condition);
    
    return 0;
}